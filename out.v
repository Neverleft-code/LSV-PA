module top (y1, y2, a, b, c);
input a, b, c;
output y1, y2;
wire g1, g2, g3;
wire t_0;
and (g1, a, b);
xor (g2, a, c);
nor (g3, b, c);
and (y1, g1, g2);
or (y2, t_0, g3);

  patch p0 ( .t_0(t_0), .g1(g1), .g2(g2) );

endmodule


// Patch   : in = 2  out = 1 : pi_in = 0  po_out = 0 : tfi = 2  tfo = 2
// Added   : gate =   1 : c0 = 0  c1 = 0  buf =  0  inv =  0  two-input =   1

module patch ( t_0, g1, g2 );

  output t_0;
  input g1, g2;
  or ( t_0, g2, g1 );

endmodule


