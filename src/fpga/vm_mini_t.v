module vm_mini_t(
			mclk, comp_ext, con_start, si_dat,
			sw_in, sw_up, sw_dn, sw_rst, sw_vref, sw_vref_n, so_dat, vref_su,
			deb_0,deb_1,deb_2
		);

input mclk,comp_ext, con_start,si_dat;
output sw_in, sw_up,sw_dn,sw_rst,sw_vref,sw_vref_n,so_dat, vref_su;
output deb_0,deb_1,deb_2;
reg[5:0] cnt;
reg[5:0] uart_divider;
reg[14:0] cnt_stat;
reg[14:0] cnt_runup;
reg[4:0] state;
reg[2:0] debug;
reg[0:0] sw_up_r, sw_dn_r, sw_rst_r, sw_vref_r, sw_in_r;
reg[0:0] runup_state_r,timer_gate_r,timer_enable_r;
reg uart_tx_start, uart_frame_start,uart_rx_rst;
reg [2:0] uart_frame_state,uart_frame_r;
reg [3:0] uart_frame_cnt;
wire uart_busy,uart_tx_bit_clock,uart_rx_bit_clock,uart_rx_ready;
reg deb;
reg comp_fast, comp_fast_sync;
reg comp_slow, comp_slow_sync;
reg cstart_slow, cstart_slow_sync;
reg [14:0] runup_cnt;
reg [14:0] runup_set;
reg [15:0] rundown_cnt;
reg rundown_sign;
reg [47:0] frame_content;
reg [47:0] result_content;
reg [7:0] uart_tx_register;
reg [2:0] uart_rx_state;
wire [7:0] uart_rx_register;

initial runup_set=1999;

	 
always @ (posedge mclk) 
	begin
	if (cnt >= 39)  
		cnt <= 0;
	else
		cnt <= cnt + 1;
   end	
		
	 
always @ (posedge cnt[5]) 
begin
	uart_divider <= uart_divider + 1;
	comp_slow_sync <= ~comp_ext;
	comp_slow <= comp_slow_sync;
	cstart_slow_sync <= con_start;
	cstart_slow <= cstart_slow_sync;

	if (uart_rx_state==0)
		begin
		if (uart_rx_ready==1)
			begin
			//do something with rx data
			if (uart_rx_register[7:4]==5)
				begin
				case (uart_rx_register[3:0])
					0 : runup_set<=199;	//1
					1 : runup_set<=399;	//2
					2 : runup_set<=599;	//3
					3 : runup_set<=799;	//4
					4 : runup_set<=999;	//5
					5 : runup_set<=1199;	//6
					6 : runup_set<=1399;	//7
					7 : runup_set<=1599;	//8
					8 : runup_set<=1799;	//9
					9 : runup_set<=1999;	//10
					default : runup_set<=1999;	//10
				endcase
				end
			uart_rx_state<=1;
			end
		end
	if (uart_rx_state==1)
		begin
		uart_rx_rst<=1;
		uart_rx_state<=2;
		end
	if (uart_rx_state==2)
		begin
		if (uart_rx_ready==0)
			begin
			uart_rx_rst<=0;
			uart_rx_state<=0;
			end
		end
		

	if (cstart_slow==0)
		begin
		state <= 19;
		end	
	if (state==0)
	begin
		if (cnt_stat==0)
			begin
			sw_rst_r <= 1;
			sw_in_r <= 0;
			sw_vref_r <= 1;
			sw_up_r <= 0;
			sw_dn_r <= 0;
			uart_frame_start <=0;	
			end
		if (cnt_stat>=3999)
			begin
			cnt_stat <= 0;
			state <= 19;
			end
		else
			begin
			cnt_stat <= cnt_stat + 1;
			end
	end

	if (state==19)
		begin
		cnt_stat <= 0;
		cnt_runup <= 0;
		runup_state_r <=0;
		runup_cnt <=0;	
		sw_rst_r <= 1;
		sw_in_r <= 0;
		sw_vref_r <= 1;
		sw_up_r <= 0;
		sw_dn_r <= 0;
		if (cstart_slow==1)
			begin
			state <= 1;
			end
		end


	if (state==1)
	begin
//		runup_set<=1999;
		uart_divider <= 0;
		uart_frame_start <=1;
		sw_rst_r <= 0;
		sw_in_r <= 1;
		sw_vref_r <= 1;
		if (runup_state_r==0)
			begin
			if (cnt_stat>=90)
				begin
				sw_up_r <= 1;
				sw_dn_r <= 0;
				end
			else
				begin
				sw_up_r <= 0;
				sw_dn_r <= 1;
				end			
			end
		else
			begin
			if (cnt_stat>=10)
				begin
				sw_up_r <= 1;
				sw_dn_r <= 0;
				end
			else
				begin
				sw_up_r <= 0;
				sw_dn_r <= 1;
				end			
			end	
		if (cnt_stat>=99)
			begin
			cnt_stat<=0;
			cnt_runup <= cnt_runup + 1;
			if (cnt_runup>=runup_set)
				begin
				state <= 2;
				end
			else
				begin
				if (comp_slow==1) 
					begin
					runup_state_r <= 1;
					runup_cnt <= runup_cnt + 1;
					end
				else runup_state_r <= 0;
				end
			end
		else
			begin
			cnt_stat <= cnt_stat + 1;
			end
	end	 
	
	if (state==2)
	begin
		sw_rst_r <= 0;
		sw_in_r <= 0;
		sw_vref_r <= 0;
		sw_up_r <= 0;
		sw_dn_r <= 0;
		if (cnt_stat>=19)
			begin
			cnt_stat<=0;
			state <= 3;
			if (comp_slow==1) runup_state_r <= 1;
			else runup_state_r <= 0;
			end
		else
			begin
			cnt_stat <= cnt_stat + 1;
			end
	end

	if (state==3)
	begin
		sw_rst_r <= 0;
		sw_in_r <= 0;
		sw_vref_r <= 0;
		timer_enable_r<=1;
		if (runup_state_r==0)
			begin
			sw_up_r <= 0;
			sw_dn_r <= 1;
			rundown_sign <= 0;
			end
		else
			begin
			sw_up_r <= 1;
			sw_dn_r <= 0;
			rundown_sign <= 1;
			end	

		if (cnt_stat>=1199)
			begin
			cnt_stat<=0;
			state <= 0;
			result_content[46:32] <= runup_cnt;
			result_content[31] <= rundown_sign;
			result_content[30:16] <= runup_set;
			result_content[15:0] <= rundown_cnt;
			end
		else
			begin
			cnt_stat <= cnt_stat + 1;
			end
	end	
	
	if (state==0) debug <= 1;
	if (state==1) debug <= 2;
	if (state==2) debug <= 4;

end

always @ (posedge mclk)
	begin
	comp_fast_sync <= ~comp_ext;
	comp_fast <= comp_fast_sync;
	if (state==2)
		begin
		rundown_cnt<=0;
		timer_gate_r<=1;
		end
	if (state==3)
		begin
		if (runup_state_r==comp_fast)
			begin
			if (timer_gate_r==1)
				rundown_cnt <= rundown_cnt + 1;
			end
		else
			begin
//			timer_gate_r<=0;
			end
		end
	end

	
assign deb_0 = si_dat;
//assign deb_1 = si_dat;
assign deb_2 = debug[0];
assign sw_up = sw_up_r;
assign sw_dn = sw_dn_r;
assign sw_rst = ~sw_rst_r;
assign sw_vref = ~sw_vref_r;
assign sw_vref_n = sw_vref_r;
assign sw_in = ~sw_in_r;
assign uart_tx_bit_clock = uart_divider[3];
assign uart_rx_bit_clock = uart_divider[0];
assign vref_su = 1;


always @ (posedge uart_tx_bit_clock)
	begin
	if (uart_frame_start)
		begin
		uart_frame_r <=1;
		end
	if ((uart_frame_r==1)&(uart_frame_start==0))
		begin
		uart_frame_r <=0;
		uart_frame_cnt <= 0;
		//frame_content <= 32'hAA550FF0;
		frame_content <= result_content;
		uart_frame_state <=0;		
		end
	if (uart_frame_state==0)
		begin
		if (uart_frame_cnt==0)
			uart_tx_register <= frame_content[47:40];
		if (uart_frame_cnt==1)
			uart_tx_register <= frame_content[39:32];
		if (uart_frame_cnt==2)
			uart_tx_register <= frame_content[31:24];
		if (uart_frame_cnt==3)
			uart_tx_register <= frame_content[23:16];
		if (uart_frame_cnt==4)
			uart_tx_register <= frame_content[15:8];
		if (uart_frame_cnt==5)
			uart_tx_register <= frame_content[7:0];
		uart_tx_start<=1;
		if (uart_busy==1)
			uart_frame_state <=1;
		end
	else if (uart_frame_state==1)
		begin
		uart_tx_start<=0;
		if (uart_busy==0)
			begin
			if (uart_frame_cnt==5)
				begin
				uart_frame_state <=2;
				end
			else
				begin
				uart_frame_cnt <= uart_frame_cnt + 1;
				uart_frame_state <=0;
				end
			end
		end
	else if (uart_frame_state==2)
		begin
		end
	end


uart_tx uart_tx_a(
.uart_tx_bclk(uart_tx_bit_clock),
.uart_tx_start(uart_tx_start),
//.uart_tx_start(uart_frame_start),
.uart_tx_data(uart_tx_register),
//.uart_tx_data(8'hA6),
.uart_tx_pin(so_dat),
.uart_tx_busy(uart_busy)
);


uart_rx uart_rx_a(
.uart_rx_bclk(uart_rx_bit_clock),
.uart_rx_pin_raw(si_dat),
.uart_rx_data(uart_rx_register),
.uart_rx_rdy(uart_rx_ready),
.uart_rx_clr(uart_rx_rst),
.uart_rx_dbg(deb_1)
);


endmodule

