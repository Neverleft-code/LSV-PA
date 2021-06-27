module top (o, a, b, c);
input a, b, c;
output o;
wire n1, n2;
wire t_0, t_1;
buf (o, t_0);
and (n2, t_1, c);
and (n1, b, c);

  patch p0 ( .t_0(t_0), .t_1(t_1), .a(a), .n1(n1) );

endmodule


// Patch   : in = 2  out = 2 : pi_in = 1  po_out = 1 : tfi = 1  tfo = 4
// Added   : gate =   2 : c0 = 1  c1 = 0  buf =  0  inv =  0  two-input =   1

module patch ( t_0, t_1, a, n1 );

  output t_0, t_1;
  input a, n1;
  or ( t_0, n1, a );
  buf ( t_1,  1'b0 );

endmodule


