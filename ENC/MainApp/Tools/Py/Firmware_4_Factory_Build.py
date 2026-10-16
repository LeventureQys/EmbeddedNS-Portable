'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

                                    Firmware_4_Factory_Build


功能描述: 生成工厂烧录的Hex文件。只是复制Keil生成的Hex文件到指定目录。

脚本版本: V1.1 (2019/5/7)
		  
		  V1.2 (2020/1/20)
		  1) py文件所在的路径，做为起始路径。而不是Keil工程所在的路径
		  2) 添加输入文件不存在的异常处理
		  
		  V1.3 (2022/8/10)
		  对输出文件名进行规范化

作    者: Tony.Chen

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
'''


# 设置参数
class Settings:
	# 1. 输入文件，路径及文件名（绝对或相对路径都可以）
	inputFile = r'..\..\Output\Release_Hex\Release.hex'

	# 2. 输出文件
	outputFile = r'..\..\Tools\Release\INNO SoftEmbe_Firmware_P0JY03(PD100Gen2) MCU'
	rev = 0

	# 3. 版本号存放文件
	versionFile = r'..\..\source\user\FirmwareUpgrade\Fw.Version.h'

	# 4. 输出文件是否加当前时间。名称的基础上，加上当前时间做为新的文件名
	#    None
	#    Time Short (yyyy.mm.dd)
	#    Time Long  (yyyy.mm.dd.hhmm)
	fileNameOutputType = 'Time Short'


# 定义函数或方法
class Func:
	verX = 0		# 固件版本号 X
	verY = 0		# 固件版本号 Y
	verZ = 0		# 固件版本号 Z
	locTime = ''

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

	# 读Hex文件，返回Hex内容
	def ReadHexFile(file):
		try:
			fin = open(file, 'r', encoding='utf-8')
			return (fin.read())
		except:
			print('Can not open the \"{}\" file!'.format(file))
			sys.exit(0)

	# 生成新的*.hex文件
	def BuildHexFile(file, hexStr):
		file = file + '.hex'

		with open(file, 'w', encoding='utf-8') as fout:
			fout.write(hexStr)

	# 转换成功后，在Keil中打印些信息
	def OutputInfo():
		print(' ')
		print('=========================== Firmware_4_Factory_Build ===========================')
		print(' ')
		print('Build succeeded.')
		print(' ')
		print('=========================== Firmware_4_Factory_Build ===========================')
		print(' ')


# 脚本主体
if __name__ == "__main__":
	import os, sys
	abs_file = os.path.abspath(__file__)
	abs_dir = abs_file[:abs_file.rfind('\\')]
	os.chdir(abs_dir)	# 转到py文件所在的路径下
	
	LocalTime = Func.GetLocalTime()
	Func.GetNewFwVersion(Settings.versionFile)
	HexString = Func.ReadHexFile(Settings.inputFile)

	FileOut = Settings.outputFile + ' V{}.{}.{}_'.format(Func.verX, Func.verY, Func.verZ) + LocalTime + '_Rev{}'.format(Settings.rev)		# 对输出文件重命名
	Func.BuildHexFile(FileOut, HexString)

	Func.OutputInfo()

