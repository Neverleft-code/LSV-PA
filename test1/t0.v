module top (o, a, b, c);
input a, b, c;
output o;
wire n1;
wire t_0;
buf (o, t_0);
and (n1, b, c);
endmodule
