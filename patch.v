// Patch   : in = 2  out = 1 : pi_in = 0  po_out = 0 : tfi = 2  tfo = 2
// Added   : gate =   1 : c0 = 0  c1 = 0  buf =  0  inv =  0  two-input =   1

module patch ( t_0, g1, g2 );

  output t_0;
  input g1, g2;
  or ( t_0, g2, g1 );

endmodule

