CONFIG_FILE PARSER:
	SAMPLE CONFIG FILE:
	block1 
	{
		directive1 directive1_arg1; directive2 directive2_arg1;
		block2 block2_arg1 block2_arg2
		{
			block3
			{
				directive3;
				block4 {}
			}
		}
		block5
		{
		}
		block6 block6_arg1{ block7 {} }
	}

	AFTER PARSE (arguments are not printed):
	root
	block1
		.directive1
		.directive2
		block2
			block3
				.directive3
				block4
		block5
		block6
			block7

