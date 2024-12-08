import telnetlib



import binascii



HOST = "192.168.8.110"

tn = telnetlib.Telnet(HOST)



#tx = tn.read_until(b"ae", 2)

tx = tn.read_some()

##tx2 = tn.read_until(b"\xF2\x5A")

## tx3 = binascii.b2a_hex(tx)

## tx2 = binascii.hexlify(tx, '-')

#tx2.fromhex(tx)
#s=binascii.unhexlify(tx)

#tx2 = hex(tx)

#tx2 = tx.encode('utf-8')



tx2 = binascii.b2a_hex(tx, b'_', 1)

# f2 5a  02 dd 98  07 45 3c  3d ae 07  46 c7 4c  df b0 78  89 a5 61  01 e1
# f2 5a  02 dd 98  07 44 96  3d ae 05  38 c9 4c  df b0 56  12 3f 61  01 2d
# f2 5a  02 dd 98  07 44 9e  3d ae 03  9e 69 4c  df b0 9f  57 b0 61  01 38

#
# f2_5a  02_dc_08  07_2a_c5  3e_17_05  34_43_4e  0c_78_48  6e_50_61  01_e7
# f2_5a  02_dc_08  07_2a_c5 (lost 00) 3e_17_05  34_43_4e  0c_78_48  6e_50_61  (lost 00) 01_e7

# correct
# F2 5A  02 DA 78  07 28 BB  00 3D 3B  04 35 B5  4C C4 58  11 9F 4F  61 00 00 6C

# output by putty 24 bytes
# F2 5A  02 DD 98  07 44 23  00 3D AE  03 A5 F7  4C DF B0  24 DD A5  61 00 01 52


print(tx2)

# print(tx.decode('ascii'))