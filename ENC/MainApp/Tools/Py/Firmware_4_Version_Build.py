import os
abs_file = os.path.abspath(__file__)
abs_dir = abs_file[:abs_file.rfind('\\')]
os.chdir(abs_dir)	# 转到py文件所在的路径下

# cmd = r'Firmware_4_Version_Build.exe -f ..\..\source\user\FirmwareUpgrade\Fw.Version.h -b'
cmd = r'Firmware_4_Version_Build.exe -f ..\..\source\user\FirmwareUpgrade\Fw.Version.h'

os.system(cmd)

