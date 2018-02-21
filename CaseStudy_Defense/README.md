# Case Study: A Novel Defense against Model Stealing
In this case study, we transfer the concept of
closeness-to-the-boundary from the area of digital watermarking to machine learning. 
In particular, we demonstrate that this defense effectively mitigates the risk of model
extraction by identifying sequences of malicious queries.

## Getting Started
1. Requirements
    - Scikit learn 0.17
    - Python 2.7

2. Download the original repository from https://github.com/ftramer/Steal-ML
    Our idea was that we add the defenses to this existing attack repository, so that
    we can easily test new attacks or defenses.

3. Put our files under the trees directory in the downloaded repository. 

4. You may have to adjust the tree_stealer.py or to replace it by our version.
    - We've slightly changed the tree_stealer.py from the Usenix paper. It should not impact
        the original implementation, but was necessary to get more information. So use some diff tool
        and replace the file if necessary.
    - We have not changed any other file.. so our code is fully compatible to the repository.

## Structure
- The tree_scikit.py is the main file where all attacks and defenses start.
    - Also look at the config_countermeasure to adjust the possible settings.
    - You will also need to tune the decision tree hyperparameters. I've tried various settings in a separate file and put them in DecisionTreeScikit.py in the init method.

- Some data files are stored in the data directory. The input must be a csv table where the label has the column name "class".
    Ensure that we only have continuous features..


## Misc.
The countermeasure is implemented via the strategy design pattern, so ideally it should be easier
to add new defenses to evaluate what works better..


