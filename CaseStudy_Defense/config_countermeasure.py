import numpy as np
## Config File ##

# The directory name where the results are saved. This dir. will be created in the same dir. as this file.
resultsdirname = "ResultsModelStealing"

# at each x query the countermeasure will check for an attack
attack_check_intervals = 10
# the minimum number of queries in a leaf before the countermeasure starts to track this leaf
MIN_COUNTED_QUERIES_IN_LEAF = 5

# controls the width of the security margin manually
margin_rate = 0.075  # margin not more than e.g. 0.2 = 1/5 of distance between threshold-left and -right
# controls the width of the security margin. It sets the width such that
# the false probability of the train sample distribution in a given dimension is below this value.
false_probability = 0.005

# the threshold where the countermeasure will block further access
threshold_blocking = 0.3

# consider ratios from leaves only where queries were recorded.
justinvolvedleaves = False


## Dataset Set-Up ##
MINFEATURE = 0 # feature range
MAXFEATURE = 1 # feature range

# if specific columns should be removed from tables
rm_cols = []
# what csv files should be used.
# data_paths = ["../data/iris.csv","./data/ISLR/Carseats.csv","./data/ISLR/College.csv","./data/ISLR/OrangeJuiceData.csv"]
data_paths = ["./data/Misc/abalone.csv","./data/Misc/winequality.csv"]
# data_paths = ["../data/iris.csv"]
# rm_cols = ["ID"]
# data_paths = ["./data/ISLR/OrangeJuiceData.csv"]

## Set-up Attacker ##
# repetitions
seedrange = range(101,106)

# for each attack query, send x cover queries
noiserange = np.array([1,2,3,4,5,10,20,40],dtype=np.int16)
# print some statistics (should be renamed, since we plotted something earlier)
showplots = True


## Set-Up Weak Attacker ##
percentageknowndata = np.array([0.1,0.2,0.3,0.4,0.5,0.99]) # np.linspace(0.1,0.5,5)
repetitions_weakatt = 5
usealltraindata = True # select a fake query from data that attacker is assumed to have
