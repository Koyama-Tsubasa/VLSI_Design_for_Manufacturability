import sys
import os
from matplotlib import pyplot as plt
from matplotlib.patches import Rectangle

input_file = sys.argv[1]

# read input file
with open("../input/"+input_file) as f:
	[chip_left, chip_bottom, chip_right, chip_top, window] = list(map(int, f.readline().split()))
	[CP_num, L_num, C_num] = list(map(int, f.readline().split()))

	# critical path id
	CP = []
	for i in range(CP_num):
		CP.append(int(f.readline()))
	
	# ignore layer information
	for i in range(L_num):
		f.readline()
	
	# conductor information
	Conductors = [[] for i in range(L_num)]
	for i in range(C_num):
		Conductor = list(map(int, f.readline().split()))
		Conductors[Conductor[6]-1].append(Conductor)

# read output file
dummy_fills = {}
for layer in range(L_num):
	dummy_fills[layer+1] = []
with open("../output/"+input_file) as f:
	for line in f.readlines():
		[left, bottom, right, top, layer] = list(map(int, line.split()))
		dummy_fills[layer].append([left, bottom, right-1, top-1])

# start draw layouts
file_name = "case_" + input_file.split('.')[0]
if not os.path.exists(file_name):
	os.makedirs(file_name)
for i in range(L_num):
	print("----- Layer", i+1, "-----")

	print("drawing the layer ...")
	plt.switch_backend('Agg')
	plt.figure(figsize=((chip_right-chip_left)/5000, (chip_top-chip_bottom)/5000))
	plt.xlim(left=chip_left)
	plt.xlim(right=chip_right)
	plt.ylim(bottom=chip_bottom)
	plt.ylim(top=chip_top)
	plt.axis('off')
	ax = plt.gca()
	for [_, left, bottom, right, top, nid, __] in Conductors[i]:
		if nid in CP:
			c = 'red'
			# rect = Rectangle((left-1600, bottom-1600), (right-left+3200), (top-bottom+3200), facecolor=c, fill=False)
			# ax.add_patch(rect)
		else:
			c = 'blue'
		rect = Rectangle((left, bottom), right-left, top-bottom, facecolor=c, fill=True)
		ax.add_patch(rect)
	for [left, bottom, right, top] in dummy_fills[i+1]:
		rect = Rectangle((left, bottom), right-left, top-bottom, facecolor='green', fill=True)
		ax.add_patch(rect)
	# for j in range(chip_left+10000, chip_right, 2500):
	# 	plt.plot([j, j], [chip_bottom, chip_top],color="orange")
	# for j in range(chip_bottom+10000, chip_top, 2500):
	# 	plt.plot([chip_left, chip_right], [j, j],color="orange")
	plt.savefig("./"+file_name+"/Layer_"+str(i+1)+".png")
	plt.close()
	print("-------------------")
	# break