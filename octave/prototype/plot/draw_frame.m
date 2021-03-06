function draw_frame(T_WS, scale=1.0)
  r_WS = tf_trans(T_WS);
	origin = r_WS;

	x_axis = T_WS * homogeneous(scale * [1; 0; 0]);
	y_axis = T_WS * homogeneous(scale * [0; 1; 0]);
	z_axis = T_WS * homogeneous(scale * [0; 0; 1]);

	% Draw x-axis
	plot3([origin(1), x_axis(1)], ...
				[origin(2), x_axis(2)], ...
				[origin(3), x_axis(3)], 'r',
				"linewidth", 5)

	% Draw y-axis
	plot3([origin(1), y_axis(1)], ...
				[origin(2), y_axis(2)], ...
				[origin(3), y_axis(3)], 'g',
				"linewidth", 5)

	% Draw z-axis
	plot3([origin(1), z_axis(1)], ...
				[origin(2), z_axis(2)], ...
				[origin(3), z_axis(3)], 'b',
				"linewidth", 5)
endfunction
