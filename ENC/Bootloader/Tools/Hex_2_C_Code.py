'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

                                          Hex_2_C_Code


功能描述: 把Hex文件的内容转换成C语言的数组（字节）。一般用于Bootloader, Dfu等Keil工程生成的Hex文
	      件，转换数组后，给MainApp Keil工程包含使用。

脚本版本: V1.1 (2019/5/7)
		  
		  V1.2 (2020/1/20)
		  1) py文件所在的路径，做为起始路径。而不是Keil工程所在的路径
		  2) 添加输入文件不存在的异常处理

作    者: Tony.Chen

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
'''


# 设置参数
class Settings:
	# 1. 输入文件，路径及文件名（绝对或相对路径都可以）
	inputFile = r'..\Output\Release\Release.hex'

	# 2. 输出文件
	outputFile = [r'.\\Fw.BlCode.c',
				 ]

	# 3. 其它信息
	programName    = 'BlHexCode'	# 程序名称（生成C文件中数组的名称）
	programAddress = 0x00000000		# 程序存放地址
	pageSize       = 1024			# 扇区/页大小（字节），用来补齐程序大小的，扇区的整数倍


# 定义函数或方法
class Func:
	dataLengthSrc = 0	# 原始Hex大小
	dataLengthNew = 0	# 补扇区整数倍后的大小

	# 返回一个Bin内容的数组
	def ReadHexFile(file):
		retArray = []
		try:
			fin = open(file, 'r')
			addrNew = 0			# 完整地址
			addrOld = 0
			is1stSegFlag = 1	# hex第1段标志，因为第1段不需要补FF
			newSegFlag   = 0	# hex分段标志，遇到新的段设为1，使用后清0

			for line in fin:
				if len(line) < 11:			# 不处理空行
					continue
				xlen = int(line[1:3], 16)
				addr = int(line[3:7], 16)	# 地址的低半字
				type = int(line[7:9], 16)

				if type == 4:				# 地址的高半字
					addrNew = (int(line[9:13], 16) << 16) & 0xFFFF0000
					if is1stSegFlag == 1:
						is1stSegFlag = 0
					else:
						newSegFlag = 1

				elif type == 0:
					addrNew = (addrNew & 0xFFFF0000) | addr

					if newSegFlag == 1:
						newSegFlag = 0
						if addrNew != addrOld and addrNew < 0x08FFFFFF:
							for i in range(0, addrNew - addrOld):
								retArray.append(255)

					for i in range(0, xlen):
						tmp = int(line[2*i+9 : 2*i+11], 16)
						retArray.append(tmp)

					addrNew += xlen
					addrOld = addrNew
		except:
			print('Can not open the \"{}\" file!'.format(file))
			sys.exit(0)

		return retArray

	# 按扇区整数倍来补FF
	def DataRepair(bArray):
		Func.dataLengthSrc = len(bArray)
		Func.dataLengthNew = int(Func.dataLengthSrc / Settings.pageSize)

		ret = bArray
		if (Func.dataLengthSrc % Settings.pageSize) > 0:
			Func.dataLengthNew += 1
			for i in range(0, Settings.pageSize - (Func.dataLengthSrc % Settings.pageSize)):
				ret.append(255)

		Func.dataLengthNew *= Settings.pageSize
		return ret

	# 生成*.c文件
	def BuildCFile(file, bArray):
		with open(file, 'w') as fout:
			s  = 'const unsigned char {}'.format(Settings.programName)
			s += '[{}*{}]'.format(int(Func.dataLengthNew/Settings.pageSize), Settings.pageSize)
			s += '__attribute__((at(0x{:0>8X}))) = '.format(Settings.programAddress)
			fout.write(s)
			fout.write('\n{\n')

			for i in range(0, len(bArray)):
				s = ''
				if i%16 == 0 and i >= 16:
					s += '\n    '
				elif i == 0:
					s += '    '

				s += '0x{:0>2X}, '.format(bArray[i])
				fout.write(s)
			fout.write('\n};\n')

	# 转换成功后，在Keil中打印些信息
	def OutputInfo():
		print(' ')
		print('================================= Hex_2_C_Code =================================')
		print(' ')
		print('Program Name:         {}'.format(Settings.programName))
		print('Program Address:      0x{:0>8x}'.format(Settings.programAddress))
		print('Flash Sector Size:    {}'.format(Settings.pageSize))
		print('Code Size:            (original / final)')
		print('                      {} / {}'.format(Func.dataLengthSrc, Func.dataLengthNew))
		print('                      0x{:x} / 0x{:x}'.format(Func.dataLengthSrc, Func.dataLengthNew))
		print(' ')
		print('================================= Hex_2_C_Code =================================')
		print(' ')


# 脚本主体
if __name__ == "__main__":
	import os, sys
	abs_file = os.path.abspath(__file__)
	abs_dir = abs_file[:abs_file.rfind('\\')]
	os.chdir(abs_dir)	# 转到py文件所在的路径下

	BinArraySrc = Func.ReadHexFile(Settings.inputFile)
	BinArrayNew = Func.DataRepair(BinArraySrc)

	for i in range(0, len(Settings.outputFile)):
		Func.BuildCFile(Settings.outputFile[i], BinArrayNew)

	Func.OutputInfo()

