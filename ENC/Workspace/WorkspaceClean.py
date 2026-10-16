import os, shutil

abs_file = os.path.abspath(__file__)
abs_dir = abs_file[:abs_file.rfind('\\')]
os.chdir(abs_dir)

projectName = r'..\\'

subProj = [
	r'Bootloader',
	r'MainApp',
]

outFolder = [
	r'Output\Debug',
	r'Output\Release',
	r'Output\Release_Dfu',
	r'Output\Release_Hex',
]

path = []
for sub in subProj:
	for out in outFolder:
		path.append(projectName + sub  + '\\' + out)

for p in path:
	try:
		shutil.rmtree(p)
	except:
		continue


