'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

                                    Firmware_4_Updater_Build


功能描述: MCU固件升级文件生成器。把Keil生成的hex文件直接转成 *.bin 二进制文件。
	      Keil编译生成Hex后，运行本Python脚本。

脚本版本:  V1.2 (2025/8/21)
		  Niko.Chen：修改打包bootloader的在线更新文件

                  V1.1 (2022/8/8)
		  对输出文件名进行规范化

		  V1.0 (2022/2/11)
		  初版

作    者: Tony.Chen

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
'''


# 设置参数
class Settings:
	# 1. 输入文件，路径及文件名（绝对或相对路径都可以）
	inputFile = r'..\..\Output\Release\Release.hex'

	# 2. 输出文件
	outputFile = r'..\..\Tools\Py\INNO SoftEmbe_Bootloader(PD100UGen2) MCU [内部使用]'
	rev = 0

	# 3. 版本号存放文件
	# versionFile = r'' 		#Bootloader取消版本设置

	# 4. Model信息 
	modelCode = 'Bootld' 	# 升级Bootloader标识
	modelName = '000000'	# 升级Bootloader没有名称
	chipID    = 0		# 系统只有单个芯片，固定为0。多个芯片，则依次累加1，
						# 务必保证不同芯片Chip ID不能相同

	# 5. Alpha协议中，固件分块发送，指的每块数据大小。
	#    256/512/1024
	dataBlockSize = 256

	# 6. 是否加密
	#    Yes, No
	encryptType = 'Yes'

	# 7. 输出文件是否加当前时间。名称的基础上，加上当前时间做为新的文件名
	#    None
	#    Time Short (yyyy.mm.dd)
	#    Time Long  (yyyy.mm.dd.hhmm)
	fileNameOutputType = 'Time Short'


# 定义函数或方法
class Func:
	verX = 0		# 固件版本号 X
	verY = 0		# 固件版本号 Y
	verZ = 0		# 固件版本号 Z
	dataLength = 0	# 固件数据长度（字节）
	dataChksum = 0	# 固件数据校验和
	frameCount = 0	# 固件打包数目

	# 获取当前系统日期和时间
	def GetLocalTime():
		import time
		yy = time.strftime('%Y',time.localtime())
		mo = time.strftime('%m',time.localtime())
		dd = time.strftime('%d',time.localtime())
		hh = time.strftime('%H',time.localtime())
		mi = time.strftime('%M',time.localtime())
		return (yy + '-' + mo + '-'  + dd)

	# 获取新的版本号
	def GetNewFwVersion(file):
		try:
			fin = open(file, 'r', encoding='utf-8')
			for line in fin:
				if line.find('FW_VERSION_X') >= 0:
					Func.verX = int(line[line.index('(')+1 : line.index(')')])
				elif line.find('FW_VERSION_Y') >= 0:
					Func.verY = int(line[line.index('(')+1 : line.index(')')])
				elif line.find('FW_VERSION_Z') >= 0:
					Func.verZ = int(line[line.index('(')+1 : line.index(')')])
		except:
			print('Can not open the \"{}\" file!'.format(file))
			sys.exit(0)

	def crc32Calc(array, crc=0):
		if len(array) % 4 > 0:
			print('注意，计算CRC32时，存在数据长度非4的整数倍')
		
		for i in range(0, int(len(array)/4)):	
			data = array[4*i] | array[4*i+1]<<8 | array[4*i+2]<<16 | array[4*i+3]<<24
			data &= 0xFFFFFFFF
			crc  += data
			crc  &= 0xFFFFFFFF
		return crc
	
	# 读Hex文件，返回一个二进制数组
	def ReadHexFile(file):
		retArray = []
		Func.dataChksum = Func.crc32Calc([])	# 初始化CRC32计算
		
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
							Func.dataLength += (addrNew - addrOld)
							for i in range(0, addrNew - addrOld):
								retArray.append(255)
							Func.dataChksum = Func.crc32Calc([255 for i in range(addrNew - addrOld)], Func.dataChksum)

					crcbuf = []
					for i in range(0, xlen):
						tmp = int(line[2*i+9 : 2*i+11], 16)
						retArray.append(tmp)
						crcbuf.append(tmp)
					
					Func.dataChksum = Func.crc32Calc(crcbuf, Func.dataChksum)
					Func.dataLength += xlen
					addrNew += xlen
					addrOld = addrNew
		except:
			print('Can not open the \"{}\" file!'.format(file))
			sys.exit(0)

		return retArray

	# 按扇区整数倍来补FF
	def DataRepair(bArray):
		ret = bArray
		if len(ret)%4 > 0:			# 补4字节对齐
			for i in range(0, 4-len(ret)%4):
				ret.append(0)
		return ret
	
	# 生成*.c文件
	def BuildCFile(file, bArray):
		Func.frameCount = int(Func.dataLength / Settings.dataBlockSize)
		if (Func.dataLength % Settings.dataBlockSize) > 0:
			Func.frameCount += 1

		file = file + '.c'
		
		with open(file, 'w', encoding='utf-8') as fout:
			# 进入DFU模式指令
			fout.write('EnterDfu[11] = \n')
			fout.write('{')
			mode = [0xC4, 0x0B, 0x00, 0x00, 0x03, 0x08, 0x10, 0x01, 0x00, 0x15, 0xFF]
			for i in range(0, len(mode)):
				s = ''
				if i%8 == 0:
					s += '\n    '
				s += '0x{:0>2X}, '.format(mode[i])
				fout.write(s)
			fout.write('\n};')

			# 固件信息帧
			head = []
			for i in range(0, 6):				# Model Code
				if i < len(Settings.modelCode):
					head.append(ord(Settings.modelCode[i : i+1]))
				else:
					head.append(0)
			for i in range(0, 64):				# Model Name
				if i < len(Settings.modelName):
					head.append(ord(Settings.modelName[i : i+1]))
				else:
					head.append(0)
			
			head.append(Func.verX)		# Firmware Version
			head.append(Func.verY)
			head.append(Func.verZ)

			head += [	(Func.dataLength      ) & 0xFF,
						(Func.dataLength >>  8) & 0xFF,
						(Func.dataLength >> 16) & 0xFF,
						(Func.dataLength >> 24) & 0xFF,
						(Func.dataChksum      ) & 0xFF,
						(Func.dataChksum >>  8) & 0xFF,
						(Func.dataChksum >> 16) & 0xFF,
						(Func.dataChksum >> 24) & 0xFF]

			headpack = Func.SigmaBusDataPack(2, Settings.chipID, head)
			fout.write('\n\n')
			fout.write('DfuInfo[{}'.format(len(headpack)) + '] = \n')
			fout.write('{')

			for i in range(0, len(headpack)):
				s = ''
				if i%8 == 0:
					s += '\n    '
				s += '0x{:0>2X}, '.format(headpack[i])
				fout.write(s)

			fout.write('\n};')

			# 固件代码块帧
			xlen = len(bArray)
			bksize = Settings.dataBlockSize
			bk = 0
			while xlen > 0:
				if xlen >= bksize:
					codePack = Func.SigmaBusDataPack(3, Settings.chipID, bArray[bk*bksize : (bk+1)*bksize])
					xlen -= bksize
				else:
					codePack = Func.SigmaBusDataPack(3, Settings.chipID, bArray[bk*bksize : len(bArray)])
					xlen = 0

				fout.write('\n\n')
				fout.write('DfuData[{}'.format(bk) + '][{}'.format(len(codePack)) + '] = \n')
				fout.write('{')

				for i in range(0, len(codePack)):
					s = ''
					if i%8 == 0:
						s += '\n    '
					s += '0x{:0>2X}, '.format(codePack[i])
					fout.write(s)

				fout.write('\n};')
				bk += 1

	# 生成*.txt文件
	def BuildTxtFile(file, bArray):
		Func.frameCount = int(Func.dataLength / Settings.dataBlockSize)
		if (Func.dataLength % Settings.dataBlockSize) > 0:
			Func.frameCount += 1

		file = file + '.txt'
		
		with open(file, 'w', encoding='utf-8') as fout:
			# 进入DFU模式指令
			mode = [0xC4, 0x0B, 0x00, 0x00, 0x03, 0x08, 0x10, 0x01, 0x00, 0x15, 0xFF]
			for i in range(0, len(mode)):
				s = ''
				if i%16 == 0 and i != 0:
					s += '\n'
				s += '{:0>2X} '.format(mode[i])
				fout.write(s)

			# 固件信息帧
			head = []
			for i in range(0, 6):				# Model Code
				if i < len(Settings.modelCode):
					head.append(ord(Settings.modelCode[i : i+1]))
				else:
					head.append(0)
			for i in range(0, 64):				# Model Name
				if i < len(Settings.modelName):
					head.append(ord(Settings.modelName[i : i+1]))
				else:
					head.append(0)
			
			head.append(Func.verX)		# Firmware Version
			head.append(Func.verY)
			head.append(Func.verZ)
			
			head += [	(Func.dataLength      ) & 0xFF,
						(Func.dataLength >>  8) & 0xFF,
						(Func.dataLength >> 16) & 0xFF,
						(Func.dataLength >> 24) & 0xFF,
						(Func.dataChksum      ) & 0xFF,
						(Func.dataChksum >>  8) & 0xFF,
						(Func.dataChksum >> 16) & 0xFF,
						(Func.dataChksum >> 24) & 0xFF]

			headpack = Func.SigmaBusDataPack(2, Settings.chipID, head)
			fout.write('\n\n')

			for i in range(0, len(headpack)):
				s = ''
				if i%16 == 0 and i != 0:
					s += '\n'
				s += '{:0>2X} '.format(headpack[i])
				fout.write(s)

			# 固件代码块帧
			xlen = len(bArray)
			bksize = Settings.dataBlockSize
			bk = 0
			while xlen > 0:
				if xlen >= bksize:
					codePack = Func.SigmaBusDataPack(3, Settings.chipID, bArray[bk*bksize : (bk+1)*bksize])
					xlen -= bksize
				else:
					codePack = Func.SigmaBusDataPack(3, Settings.chipID, bArray[bk*bksize : len(bArray)])
					xlen = 0

				fout.write('\n\n')

				for i in range(0, len(codePack)):
					s = ''
					if i%16 == 0 and i != 0:
						s += '\n'
					s += '{:0>2X} '.format(codePack[i])
					fout.write(s)
				bk += 1

	# 生成*.bin文件
	def BuildBinFile(file, bArray):
		file = file + '.bin'
		
		with open(file, 'wb') as fout:
			# 进入DFU模式指令
			buf = [0xC4, 0x0B, 0x00, 0x00, 0x03, 0x08, 0x10, 0x01, 0x00, 0x15, 0xFF]
			fout.write(bytes(buf))

			# 固件信息帧
			head = []
			for i in range(0, 6):				# Model Code
				if i < len(Settings.modelCode):
					head.append(ord(Settings.modelCode[i : i+1]))
				else:
					head.append(0)
			for i in range(0, 64):				# Model Name
				if i < len(Settings.modelName):
					head.append(ord(Settings.modelName[i : i+1]))
				else:
					head.append(0)
			
			head.append(Func.verX)		# Firmware Version
			head.append(Func.verY)
			head.append(Func.verZ)

			head += [	(Func.dataLength      ) & 0xFF,
						(Func.dataLength >>  8) & 0xFF,
						(Func.dataLength >> 16) & 0xFF,
						(Func.dataLength >> 24) & 0xFF,
						(Func.dataChksum      ) & 0xFF,
						(Func.dataChksum >>  8) & 0xFF,
						(Func.dataChksum >> 16) & 0xFF,
						(Func.dataChksum >> 24) & 0xFF]

			headpack = Func.SigmaBusDataPack(2, Settings.chipID, head)
			fout.write(bytes(headpack))

			# 固件代码块帧
			xlen = len(bArray)
			bksize = Settings.dataBlockSize
			bk = 0
			while xlen > 0:
				if xlen >= bksize:
					codePack = Func.SigmaBusDataPack(3, Settings.chipID, bArray[bk*bksize : (bk+1)*bksize])
					xlen -= bksize
				else:
					codePack = Func.SigmaBusDataPack(3, Settings.chipID, bArray[bk*bksize : len(bArray)])
					xlen = 0

				fout.write(bytes(codePack))
				bk += 1

	# 伽马加密算法，要加密的数据，4字节的整数倍。如果不满足的，可以用00来补齐。
	def GammaEncrypt(bArray):
		key = 0x1C9CBB63		# 密钥

		if Settings.encryptType == 'Yes':
			ret = bArray

			if len(bArray) <= 8*1024:		# 只对前8K数据进行加密
				max = int(len(bArray)/4)
			else:
				max = int(8*1024/4)

			cnt = 0
			for i in range(0, max):
				for j in range(0, 4):
					tmp = ret[cnt]
					tmp = tmp ^ ((key >> (8 * j)) & 0xFF)

					msb = (tmp << 4) & 0xF0
					lsb = (tmp >> 4) & 0x0F

					ret[cnt] = ((msb | lsb) + 0xA3) & 0xFF
					cnt += 1

			return ret
		else:
			return bArray

	# 把数据打包成Alpha协议帧
	def SigmaBusDataPack(dfuCmd, chipID, data):
		ret = []
		length = len(data)

		ret.append(0xC4)						# CP_BEGIN
		ret.append( (length + 9) & 0xFF)		# CP_LEN_L
		ret.append(((length + 9) >> 8) & 0xFF)	# CP_LEN_H
		ret.append(0x00)						# CP_ADDR
		ret.append(0x0B)						# CP_CMD
		ret.append(dfuCmd)						# DFU_CMD
		ret.append(chipID)						# CHIP_ID

		for i in range(0, length):
			ret.append(data[i])

		crc = 0
		for i in range(0, len(ret)):
			crc += ret[i]
		crc &= 0xFFFF
		crc = ~crc + 1
		crc &= 0xFFFF
		ret.append( crc & 0xFF)
		ret.append((crc >> 8) & 0xFF)
		return ret	

	# 转换成功后，在Keil中打印些信息
	def OutputInfo():
		print(' ')
		print('=========================== Firmware_4_Updater_Build ===========================')
		print(' ')
		print('Model Code              {}'.format(Settings.modelCode))
		print('Model Name              {}'.format(Settings.modelName))
		print('Firmware Version        {}'.format(Func.verX) + '.{}'.format(Func.verY) + '.{}'.format(Func.verZ))
		print('Firmware Message Numer  {}'.format(Func.frameCount))
		print('Firmware Code Size      {}'.format(Func.dataLength) + ' ({:.2f}'.format(Func.dataLength/1024.0) + 'kB, 0x{:x}'.format(Func.dataLength) + ')')
		print('Firmware Checksum       0x{:0>8x}'.format(Func.dataChksum))
		print(' ')
		print('=========================== Firmware_4_Updater_Build ===========================')
		print(' ')


# 脚本主体
if __name__ == "__main__":
	import os, sys
	abs_file = os.path.abspath(__file__)
	abs_dir = abs_file[:abs_file.rfind('\\')]
	os.chdir(abs_dir)	# 转到py文件所在的路径下
	
	LocalTime = Func.GetLocalTime()
	
	# Bootloader 取消设置版本
	# Func.GetNewFwVersion(Settings.versionFile)

	BinArray        = Func.ReadHexFile(Settings.inputFile)
	BinArray        = Func.DataRepair(BinArray)
	EncryptBinArray = Func.GammaEncrypt(BinArray)


	# FileOut = Settings.outputFile + ' V{}.{}.{}_'.format(Func.verX, Func.verY, Func.verZ) + LocalTime + '_Rev{}'.format(Settings.rev)		# 对输出文件重命名
	FileOut = Settings.outputFile		# 对输出文件重命名
	# Func.BuildCFile(FileOut, EncryptBinArray)
	# Func.BuildTxtFile(FileOut, EncryptBinArray)
	Func.BuildBinFile(FileOut, EncryptBinArray)

	Func.OutputInfo()

