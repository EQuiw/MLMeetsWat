#!/usr/bin/env python
# -*- coding: utf-8 -*-

from trees.tree_scikit_stealer import *
from trees.tree_scikit_stealer_weakattacker import *

import pandas as pd
import numpy as np
import timeit
import os

import counterm_boundary as counterm3
from DecisionTreeScikit import *
from sklearn.cross_validation import train_test_split
from sklearn.cross_validation import ShuffleSplit

import config_countermeasure as config

class TreeExtractionSetup:
    """
    This file and class is the main file responsible for the different attacks and defenses..
    Instead of using BigML, I learn a local model with scikit-learn..
    """

    def __init__(self, data_path, theseed=101):
        """
        Constructor of Custom Tree Extraction Setup.
        :param data_path: the path to the csv file, relative to this file.
        :param theseed: a seed for random numbers
        """
        # self.black_box = True
        self.epsilon = 0.001
        self.rounding = None

        # used for test set evaluation:
        self.accuracy_on_testset = 0.0

        # read training data
        dir_path = os.path.dirname(os.path.realpath(__file__))
        train = pd.read_csv(os.path.join(dir_path, data_path),
                    header=0, quoting=3)
        my_cols1 = set(train.columns)
        for lab in config.rm_cols:
            my_cols1.remove(lab)
        my_cols1.remove("class")
        x_train = train[list(my_cols1)]
        y_train = train["class"]

        # Split into training and test set:
        # on training set = countermeasure / tree learned;
        # on test set = benign user queries simulated
        self.x_train, self.x_test, self.y_train, self.y_test = train_test_split(x_train, y_train, test_size=0.5,
                                                                                    random_state=theseed) # 41 to reproduce test cases

        self.dataname = data_path.split("/")[-1].split(".")[0].replace("_","")
        self.seedfortraintestsplit = theseed


    def __prepare_for_evaluation(self, countermeas=None):
        dectree = DecisionTreeScikit(self.x_train, self.y_train)
        dectree.set_countermeasure_strategy(countermeas)
        return dectree


    def pprint(self, decisiontree):
        """
        Prints the current tree to disk, to project head directory.
        Then call dot -Tpng tree.dot -o tree.png
        """
        dir_path = os.path.dirname(os.path.realpath(__file__))
        with open(os.path.join(dir_path, "../tree.dot"), 'w') as f:
            f = tree.export_graphviz(decisiontree.clf, out_file=f)


    def export_summary(self, addnoise, stats, header, attacktype):
        """
        Returns a summary of statistics
        :param addnoise:
        :param stats:
        :param header:
        :param attacktype:
        :return: Saves file on disk, function itself is void
        """
        dir_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), config.resultsdirname, self.dataname)
        if not os.path.exists(dir_path):
            os.makedirs(dir_path)

        amountofnoisestr = str(addnoise).replace(".", "")
        exportstr = amountofnoisestr + "_" + self.dataname + "_" + attacktype
        # stats: Q, time, % of learned tree
        statsnp = np.array( stats )
        # reshape stats so that numpy writes the array row-wise in text file
        statsnp = statsnp.reshape(1, statsnp.shape[0])
        exportfile = os.path.join(dir_path, "stats_" + exportstr + ".dat")
        if os.path.isfile(exportfile):
            with open(exportfile, 'a') as f_handle:
                np.savetxt(f_handle, statsnp)
        else:
            np.savetxt(exportfile, statsnp, delimiter = "\t", header = header, comments = "")


    def evaluate_on_data_set(self, countermeas, y_test, x_test, eval_type):
        """
        Converts each record from the given data set into a query
        and checks how many queries are detected by the countermeasure. It also checks the accuracy
        of the decision tree on this set.
        This method is usable to evaluate honest user queries.
        -------
        """
        # I. Learn tree
        decisiontree = self.__prepare_for_evaluation(countermeas=countermeas)

        # II. Do extraction
        # create new queries from test set and check the reaction of the countermeasure
        numberofqueriesinmargin = 0
        y_predicted = [None] * y_test.shape[0]

        # Additionally check the leaf coverage of the data set. For instance, we may have 10 leaves, and all queries
        # always end in the same 5 leaves. Then, we would have a leaf coverage of 5 / 10.
        unique_leaves = {}

        # run...
        kk = 0
        for index, row in x_test.iterrows():
            query = {feature.name: row[feature.name] for feature in decisiontree.get_features()}

            # short check if query contains all features
            if len(query) != len(decisiontree.get_features()):
                raise AttributeError()

            res_id = decisiontree.make_prediction(query)
            if decisiontree.countermeasure.check_if_query_is_malicious(query, res_id):
                numberofqueriesinmargin += 1

            if unique_leaves.has_key(res_id):
                unique_leaves[res_id] += 1
            else:
                unique_leaves[res_id] = 1

            if decisiontree.last_predict.shape[0] > 1:
                raise Exception("Shape")
            y_predicted[kk] = decisiontree.last_predict[0]
            kk += 1
        accuracy_on_testset = np.sum(np.array(y_predicted) == np.array(y_test))/float(y_test.shape[0])


        # III. Export results / Evaluation
        print('----> {} Accurary:{}'.format(eval_type, accuracy_on_testset))
        print('----> {} In margin ratio:{}'.format(eval_type, numberofqueriesinmargin/float(decisiontree.countermeasure.global_query_count)))


        decisiontree.countermeasure.detect_attack()
        decisiontree.countermeasure.pprint(False)
        # exportstr = str(self.seedfortraintestsplit) + "_" + "no_noise" + "_" + self.dataname + "_test"
        # decisiontree.countermeasure.exportpointstodisk(exportname=exportstr, dirname=self.dataname,
        #                                             accuracy_on_testset=accuracy_on_testset)

        stat1 = decisiontree.countermeasure.global_query_count
        stat2 = decisiontree.countermeasure.get_last_ratio()
        stat3 = accuracy_on_testset
        stat4 = len(decisiontree.get_leaf_ids())
        # check how many leaves the data are representing
        stat5 = len(unique_leaves)/float(len(decisiontree.idx))
        stattemp = [val for val in unique_leaves.itervalues()]
        stat6 = np.mean(stattemp)
        stat7 = np.var(stattemp)
        self.export_summary(addnoise="", stats=[stat1, stat2, stat3, stat4, stat5, stat6, stat7],
                            header=" Q \t ratio \t acc \t leaves \t leaf_coverage \t leaf_cov_mean \t leaf_cov_var",
                            attacktype=eval_type)

        print('----> {} End-Ratio:{}'.format(eval_type, stat2))
        print('----> {} Leaf Coverage:{} / Mean:{} / Var:{}'.format(eval_type, stat5, stat6, stat7))
        if stat5 > x_test.shape[0]/float(len(decisiontree.idx)):
            raise Exception("Some error... look at the values...")


    def evaluate_attack_on_train_set(self, countermeas, seed, addnoise, attackmode):
        """
        Extracts the decision tree.
        :param countermeas: the used countermeasure
        :param seed: a seed
        :param addnoise: an integer specifying the number of cover queries for each attack query
        :param attackmode: if 1: The standard TreeScikitExtractor from Usenix paper is used.
        """

        # I. Learn tree
        decisiontree = self.__prepare_for_evaluation(countermeas = countermeas)

        # prepare extraction
        if attackmode==1:
            treeextractor = TreeScikitExtractor(decisiontree, self.epsilon, self.rounding, addnoise)
        else:
            raise NotImplementedError("attack on train set error. wrong attack mode")

        # II. Do extraction
        start_time = timeit.default_timer()

        try:
            np.random.seed(seed)
            all_leaves = treeextractor.extract(incomplete_queries=False)
        except counterm3.BlockedException:
            all_leaves = treeextractor.get_leaves_after_exception()
        # all_leaves = ext.extract_top_down()

        end_time = timeit.default_timer()

        # print('found {} leaves ({} unique) / {} exist'.format(
        #     sum([len(x) for x in all_leaves.values()]), len(all_leaves), len(decisiontree.idx)))
        # print('Extraction required {} queries'.format(treeextractor.queryCount))
        # print('Extraction required {} noise queries'.format(treeextractor.noisyqueryCount))
        # print('Extraction Noise Ratio: {}'.format(treeextractor.noisyqueryCount / float(treeextractor.queryCount)))
        # print('Extraction took %.2f seconds' % (end_time - start_time))

        decisiontree.countermeasure.pprint(config.showplots)

        # III. Export
        # amountofnoisestr = str(addnoise).replace(".", "")
        # exportstr = str(seed) + "_" + amountofnoisestr + "_" + self.dataname + "_attack"
        # decisiontree.countermeasure.exportpointstodisk(exportname=exportstr, dirname=self.dataname,
        #                                                accuracy_on_testset=0.0)

        ## export just some stats, not full ratio history
        stat1 = decisiontree.countermeasure.global_query_count
        stat2 = (end_time - start_time)
        stat3 = len(all_leaves) / float(len(decisiontree.idx))
        # noise in margin:
        if float(treeextractor.noisyqueryCount) != 0:
            stat4 = treeextractor.noisyqueriesinmargin / float(treeextractor.noisyqueryCount)
        else:
            stat4 = 0
        # att queries in margin:
        stat5 = treeextractor.attqueriesinmargin / float(treeextractor.queryCount)

        self.export_summary(addnoise=addnoise, stats=[stat1,stat2,stat3,stat4,stat5], header=" Q \t t \t p \t Qn \t Qa",
                            attacktype=str(countermeas.reaction)+"original_attack"+str(attackmode))




    def evaluate_on_weak_att_data_set(self, countermeas, seed, percentageknowndata, repetitions):
        """
        :param countermeas: the wanted countermeasure.
        :param seed: a seed
        :param percentageknowndata: an array that indices the fractions what the attacker knows.
        :param repetitions: used for ShuffleSplit. We split the training set into a set what the attacker knows,
        and what the defender uses for the model.
        """
        # I. Iterate over % attacker knowledge
        r = 0
        for perc in percentageknowndata:
            c = 0
            # II. Iterate over repetitions
            # Shuffle Split on x% of attacker knowledge
            rs = ShuffleSplit(self.x_train.shape[0], n_iter=repetitions, test_size=(1 - perc),
                              random_state=seed + 1)
            for train_index, test_index in rs:
                print("***{}:{}:".format(r, c))

                if isinstance(countermeas, counterm3.CounterBoundary):
                    mycountermeasure = counterm3.CounterBoundary()
                else:
                    raise Exception("Please add the wanted countermeasure here explicitly")

                # check what the attacker actually knows
                surrogate_dataset = self.x_train.iloc[train_index,]
                self.evaluate_on_data_set(countermeas=mycountermeasure, eval_type="hold_out"+str(r)+"_"+str(c)+"_train_weakatt",
                                               x_test=surrogate_dataset, y_test=self.y_train.iloc[train_index,])
                c += 1
            r += 1



    def evaluate_weak_attacker(self, countermeas, seed, addnoise, percentageknowndata, repetitions=1):
        """
        Represents an informed, but weak, attacker who knows a certain percentage of the training data.
        These known samples are usable to create better cover queries.
        Result is a table with rows = percentageknowndata, columns = repetitions,
        values = the security ratio at the end of attack
        :param countermeas: the used countermeasure
        :param seed: a seed
        :param addnoise: an integer specifying the number of cover queries for each attack query
        :param percentageknowndata: an iterable element with floats determining the percentage of known training samples
        :param repetitions:
        """

        columns = ["Rep"+str(rep) for rep in range(1,repetitions+1)]
        rows = percentageknowndata
        result_table = pd.DataFrame(index = rows, columns = columns)

        # I. Iterate over % attacker knowledge
        r = 0
        for perc in percentageknowndata:
            c = 0
            # II. Iterate over repetitions
            # Shuffle Split on x% of attacker knowledge
            rs = ShuffleSplit(self.x_train.shape[0], n_iter=repetitions, test_size = (1-perc),
                              random_state = seed+1)
            for train_index, test_index in rs:
                print("***{}:{}:".format(r,c))

                # a.1 create new countermeasure object -- necessary to reset the state for each sub-experiment
                # we should probably use a factory here for generating the countermeasuers... todo
                if isinstance(countermeas, counterm3.CounterBoundary):
                    mycountermeasure = counterm3.CounterBoundary(reaction=countermeas.reaction,
                                                                 threshold=countermeas.threshold)
                else:
                    raise Exception("Please add the wanted countermeasure here explicitly")

                # a.2 Set tree
                decisiontree = self.__prepare_for_evaluation(countermeas=mycountermeasure)

                # b. Attack
                surrogate_dataset = self.x_train.iloc[train_index,]
                treeextractor = TreeScikitWeakAttackerExtractor(decisiontree, surrogate_dataset, self.epsilon, self.rounding, addnoise)

                # c. Do extraction
                start_time = timeit.default_timer()

                try:
                    np.random.seed(seed)
                    all_leaves = treeextractor.extract(incomplete_queries=False)
                except counterm3.BlockedException:
                    all_leaves = treeextractor.get_leaves_after_exception()
                    print("Blocked")

                end_time = timeit.default_timer()

                # d. Show results
                # print('found {} leaves ({} unique) / {} exist'.format(
                #     sum([len(x) for x in all_leaves.values()]), len(all_leaves), len(decisiontree.idx)))
                # print('Extraction required {} queries'.format(treeextractor.queryCount))
                # print('Extraction required {} noise queries'.format(treeextractor.noisyqueryCount))
                # print('Extraction Noise Ratio: {}'.format(treeextractor.noisyqueryCount / float(treeextractor.queryCount)))
                # print('Extraction took %.2f seconds' % (end_time - start_time))
                decisiontree.countermeasure.pprint(config.showplots)
                print('Queries:({},{}); NoiseInMargin:{} , AttInMargin: {} , total n-quer.:{} , total a-quer.:{}'.format(r,c,
                                                                                      treeextractor.noisyqueriesinmargin/float(treeextractor.noisyqueryCount),
                                                                                      treeextractor.attqueriesinmargin/float(treeextractor.queryCount),
                                                                                                        treeextractor.noisyqueryCount, treeextractor.queryCount))

                # e. get results
                # result_table.iloc[r, c] = decisiontree.countermeasure.get_last_ratio()
                result_table.iloc[r, c] = len(all_leaves) / float(len(decisiontree.idx))
                print('Recovery:{}'.format(result_table.iloc[r, c]))

                c += 1
            r += 1

        # III. Store results to disk
        print(result_table)
        dir_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), config.resultsdirname, self.dataname)
        if not os.path.exists(dir_path):
            os.makedirs(dir_path)
        amountofnoisestr = str(addnoise).replace(".", "")
        exportstr = str(seed) + "_" + amountofnoisestr + "_" + self.dataname + "_" + countermeas.reaction+"_weak_attack"
        result_table.to_csv(path_or_buf=os.path.join(dir_path, "ratiotable_" + exportstr + ".dat"))




def main(data_path, seed):
    """ Runs the main steps of the tree extraction attack """

    # I. Init attack with countermeasure if wanted
    threshold = config.threshold_blocking
    print("Threshold:", threshold)
    treesetup = TreeExtractionSetup(data_path=data_path, theseed=seed)

    # II. Evaluate on train / test test
    # Evaluate the train set as well to get an intuition about the queries and their position.
    # Use the test set as benign input sequence and evaluate how many queries
    # are marked as malicious by the countermeasure = false positive:
    treesetup.evaluate_on_data_set(countermeas=counterm3.CounterBoundary(threshold=threshold),
                                   eval_type="hold_out_train", x_test=treesetup.x_train, y_test=treesetup.y_train)

    treesetup.evaluate_on_weak_att_data_set(countermeas=counterm3.CounterBoundary(threshold=threshold),
                                            percentageknowndata=config.percentageknowndata,
                                            repetitions=config.repetitions_weakatt, seed=seed)

    treesetup.evaluate_on_data_set(countermeas=counterm3.CounterBoundary(threshold=threshold),
                                   eval_type="hold_out_test", x_test=treesetup.x_test, y_test=treesetup.y_test)


    # III. Various attack + defense experiments with the original attack
    mycountermeasure = counterm3.CounterBoundary(threshold=threshold)
    treesetup.evaluate_attack_on_train_set(countermeas=mycountermeasure, seed=seed, addnoise=0, attackmode=1)

    # Reactions after detecting an attack: We block the further access, so attack stops.
    mycountermeasure = counterm3.CounterBoundary(threshold=threshold, reaction="Block")
    treesetup.evaluate_attack_on_train_set(countermeas=mycountermeasure, seed=seed, addnoise=0, attackmode=1)

    # Or we return random decisions -- however, the attack may not terminate in this case...
    # mycountermeasure = counterm3.CounterBoundary(threshold=threshold, reaction="RandomLeaf")
    # treesetup.evaluate_attack_on_train_set(countermeas=mycountermeasure, seed=seed, addnoise=0)
    # random leaf, the attack does not terminate, since line search is based on leaf id


    # IV. Attack our defense, with 'cover queries'
    for amount_noise in config.noiserange:
        print("Noise:", amount_noise)
        # No A Priori Knowledge attacker, uniform random noise
        mycountermeasure = counterm3.CounterBoundary(threshold=threshold, reaction="Block")
        treesetup.evaluate_attack_on_train_set(countermeas=mycountermeasure, seed=seed, addnoise=amount_noise, attackmode=1)


        # Surrogate Dataset attacker
        mycountermeasure = counterm3.CounterBoundary(threshold=threshold, reaction="Block")
        treesetup.evaluate_weak_attacker(countermeas=mycountermeasure, seed=seed, addnoise=amount_noise,
                                     percentageknowndata=config.percentageknowndata,
                                     repetitions=config.repetitions_weakatt)




if __name__ == "__main__":
    # For each data input:
    for cdata_path in config.data_paths:
        # Use various seeds:
        for cseed in config.seedrange:
            # Perform the respective experiments
            main(data_path=cdata_path, seed=cseed)

