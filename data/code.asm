		DADDI R1 R0 #20
		DADD R2 R0 R0
		DADD R3 R0 R0
		DADD R4 R0 R0
TARGET1:LD   R5 0(R4)
		DSUB R6 R5 R3
		BLTZ R6 TARGET2
		DADD R3 R5 R0
TARGET2:DADDI R4 R4 #4
		DADDI R2 R2 #1
		DSUB R6 R1 R2
		BNEZ R6 TARGET1
		SD	 R3 0(R4)
