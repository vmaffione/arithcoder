import sys;

bugenc = open( "bsenc", "r" );
bugdec = open( "bsdec", "r" );

#rc_enc = 0;
#rc_dec = 0;
cc_enc = 0;
cc_dec = 0;

while 1:
	bitenc = bugenc.read( 1 );
	bitdec = bugdec.read( 1 );
	
	if bitdec == "":
		break;
		
	'''
	if bitenc == "d":
		bugenc.readline();
		rc_enc += 1;
	if bitdec == "d":
		bugdec.readline();
		rc_dec += 1;
	
	if bitenc == "\n":
		rc_enc += 1;
		cc_enc = 0;
	if bitdec == "\n":
		rc_dec += 1;
		cc_dec = 0;
	'''
	#print(bitenc,bitdec)
	if bitenc != bitdec:
		print( "fail at", cc_dec );
		break;
	else:
		cc_enc += 1;
		cc_dec += 1;
	

bugenc.close();
bugdec.close();
