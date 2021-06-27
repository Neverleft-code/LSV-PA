module top (o, a, b, c);
input a, b, c;
output o;
wire n1, n2;
wire t_0, t_1;
buf (o, t_0);
and (n2, t_1, c);
and (n1, b, c);
endmodule
