import numpy as np
## Config File ##
resultsdirname = "ResultsModelStealing" # will be created under the trees directory.

attack_check_intervals = 10
MIN_COUNTED_QUERIES_IN_LEAF = 5
margin_rate = 0.075  # margin not more than e.g. 0.2 = 1/5 of distance between threshold-left and -right
false_probability = 0.005
threshold_blocking = 0.3

justinvolvedleaves = False

### Dataset Set-Up ###
MINFEATURE = 0
MAXFEATURE = 1
rm_cols = [] # if specific columns should be removed from tables
# data_paths = ["../data/iris.csv","./data/ISLR/Carseats.csv","./data/ISLR/College.csv","./data/ISLR/OrangeJuiceData.csv"]
data_paths = ["./data/Misc/abalone.csv","./data/Misc/winequality.csv"]
# data_paths = ["../data/iris.csv"]
# rm_cols = ["ID"]
# data_paths = ["./data/ISLR/OrangeJuiceData.csv"]


seedrange = range(101,106)  # 101 is my default seed for test cases
noiserange = np.array([1,2,3,4,5,10,20,40],dtype=np.int16)  #np.linspace(5,20,20/5,dtype=np.int16)
showplots = True
### Set-Up End ###

## Set-Up Weak Attacker ##
percentageknowndata = np.array([0.1,0.2,0.3,0.4,0.5,0.99]) # np.linspace(0.1,0.5,5)
repetitions_weakatt = 5
usealltraindata = True