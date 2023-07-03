import numpy as np
from struct import unpack as sunpack
from functools import partial
from scipy.linalg import lstsq

# TODO train actual NN instead of simple line fitting
if False:
	import tensorflow

# split chunk into board position and metadata
# TODO remmove DEPRICATED saving format
def load_board(chunk):
		board = bytearray(chunk[:16])

		[rating] = sunpack("f",chunk[16:])
		return (board,rating)

# split data into inputs and outputs for training
def load_io_data(chunk):
		inputs:int = sunpack("B"*25,chunk[:25])
		[output] = sunpack("f",chunk[25:])
		return (inputs,output)

# load an entire file of positions
# file format:
# {byte[25]:heuristics/inputs,float32:output}
def load_file(file_name="data/data"):
	inputs = []
	outputs = []
	with open(file_name,"rb") as file:
		for chunk in iter(partial(file.read, 29), b''):
			input,ouput = load_io_data(chunk)
			inputs.append(input)
			outputs.append(ouput)
	return (np.array(inputs,np.uint8),np.array(outputs,np.float32))

#find best fit line
# TODO train actual NN instead of simple line fitting
def find_best_fit_line(file_name = "data/data"):
	input_data, output_data = load_file(file_name)
	input_data_aug = np.hstack((input_data,np.ones((input_data.shape[0],1))))
	output_data_zeroed = output_data - np.average(output_data)
	fit, residual, rnk, s = lstsq(input_data_aug,output_data_zeroed,cond=1e-12)
	print("min = ",min(output_data_zeroed))
	print("max = ",max(output_data_zeroed))
	print("avg = ",np.average(output_data_zeroed))
	for param in fit:
		print(param/2)
	print(input_data_aug[0])
	print(output_data_zeroed[0])
	# TODO automate the transfer of weights to the the model
	# currently the weights have to be transfered by hand

# TODO implement NN training
# currently unused
def train_network_test():
	TRAINING_DATA_SET_PERCENTAGE = 0.8
	input_data, output_data = load_file()
	data_set_size = input_data.shape[0]
	input_data_aug = np.hstack((input_data,np.ones((input_data.shape[0],1))))
	

	# separate data set	
	x_train = input_data[:int(data_set_size*TRAINING_DATA_SET_PERCENTAGE)]
	y_train = output_data[:int(data_set_size*TRAINING_DATA_SET_PERCENTAGE)]
	x_test = input_data[int(data_set_size*TRAINING_DATA_SET_PERCENTAGE):]
	y_test = output_data[int(data_set_size*TRAINING_DATA_SET_PERCENTAGE):]


