module top (o, a, b, c);
input a, b, c;
output o;
wire n1;
or (o, a, n1);
and (n1, b, c);
endmodule
