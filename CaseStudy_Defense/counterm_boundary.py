from __future__ import division

import os
import numpy as np
import scipy as sc
from trees.countermeasures import Countermeasure
from sklearn.neighbors import KernelDensity
from sklearn.grid_search import GridSearchCV
import matplotlib.pyplot as plt
import config_countermeasure as config


class BlockedException(Exception):
    """An Exception if countermeasure blocks the current user due to malicious queries"""
    pass


class CounterBoundary(Countermeasure):
    """
    The closeness-to-the-boundary countermeasure..
    """

    def __init__(self, threshold = None, reaction=None):
        """
        Inits...
        """
        # each leaf will have an own boundary object that creates a security margin at all predicates on its way
        # then later in store_query, get the corresponding leaf's boundary object with id and look if val falls into
        # security_margin
        self.leaf_boundary = {}

        # just for pretty printing and export
        self.points = []
        self.ratios = []
        self.tree = None

        # Parameters for reaction. What reaction should be performed?
        self.threshold = threshold
        super(CounterBoundary, self).__init__(reaction = reaction)


    def set_tree(self, tree):
        self.tree = tree

        # In order to estimate the margin of the security margin in each region, we need the distribution of the
        # the training samples
        training_leaf_ids = self.tree.get_leaf_ids_from_training_data()
        leaf_paths = self.tree.idx_path

        # create a dictionary for features that stores the respective feature range
        feature_boundary_dict = {}
        for feat in self.tree.get_features():
            feature_boundary_dict[feat.name] = (feat.min_val, feat.max_val)

        # Now run over each leaf = region, and create its security margins
        for idxx in self.tree.get_leaf_ids():
            relevant_training_data_for_leaf = self.__get_training_data_for_leaf(training_leaf_ids, idxx)
            self.leaf_boundary[idxx] = LeafSecurityMargin(idxx, leaf_paths[idxx],
                                                          relevant_training_data_for_leaf, feature_boundary_dict)



    def __get_training_data_for_leaf(self, training_leaf_ids, leaf_id):
        return self.tree.x_train.iloc[training_leaf_ids == leaf_id, : ]


    def store_query(self, query, idx=None):
        """
        Stores the query feature values into the counter data structure
        -------
        """
        # store the query in the respective leaf's security object
        self.leaf_boundary[idx].store_query(query)
        self.global_query_count += 1

        # if a certain number of queries happened, check if attack happened
        if self.global_query_count % config.attack_check_intervals == 0:
            self.detect_attack()

        # just pretty 2d plotting
        queryasdatastructure = []
        for feat in self.tree.get_features():
            cval = query[feat.name]
            queryasdatastructure.append(cval)
        self.points.append(queryasdatastructure)


    def check_if_query_is_malicious(self, query, idx):
        return self.leaf_boundary[idx].check_if_query_in_margin(query)


    def detect_attack(self):
        """
        Count the number of queries inside the security margin of a particular leaf compared to the total number of
        queries in that leaf.
        """
        #        TODO rename in update_detection_statistics?
        # check the last time when detect attack was called, get ratios only if new queries were reported meanwhile
        if self.last_query_count_when_detect_attack_called < self.global_query_count:
            self.last_query_count_when_detect_attack_called = self.global_query_count

            exportratio = 0
            numberofinvolvedleaves = 0
            # iterate over each leaf and get report
            for leaf_idx in self.leaf_boundary:
                # print "Report for leaf id:" + str(leaf_idx)
                ratio = self.leaf_boundary[leaf_idx].detect_attack()
                # print "\tRatio:" + str(ratio)
                exportratio += ratio
                # Option to just consider ratios where queries were recorded at all
                if config.justinvolvedleaves:
                    if ratio > 0:
                        numberofinvolvedleaves += 1
                else:
                    numberofinvolvedleaves += 1

            if numberofinvolvedleaves > 0:
                exportratio = exportratio / numberofinvolvedleaves

            # document the current ratio in the history
            self.ratios.append(exportratio)

            # If ratio exceeds a certain threshold, this countermeasure assumes that an attack happens.
            # Tree can then take actions.
            if self.reaction is not None and self.threshold is not None and exportratio > self.threshold:
                self.detected_attack = True


    def let_countermeasure_decide(self, cleaf_id):
        """
        Returns what the tree should do based on the countermeasure's information
        """
        if self.detected_attack is True:
            if self.reaction == "Block":
                raise BlockedException("Access blocked")
            elif self.reaction == "RandomLeaf":
                # Return random leaf
                return np.random.choice(self.tree.get_leaf_ids(), 1)[0]
            else:
                raise NotImplementedError()

        # if no attack happens, return the correct leaf
        return cleaf_id


    def pprint(self, showplot = False):
        """
        A pretty print of counts
        -------
        """
        if showplot:
            # plt.figure(1, figsize=(8, 6))
            # plt.clf()
            #
            # # Plot the training points
            # X = np.array(self.points)
            # plt.scatter(X[:, 0], X[:, 1])
            # plt.show()

            # Print the ratios
            print("### Ratios: ",self.ratios[-1])

            # Per leaf: end ratio
            ratioperleaf = {}
            ratioperleaf_arr = np.zeros(len(self.leaf_boundary))
            ix = 0
            for leaf_idx in self.leaf_boundary:
                # print "Report for leaf id:" + str(leaf_idx)
                ratioperleaf[leaf_idx] = self.leaf_boundary[leaf_idx].detect_attack()
                ratioperleaf_arr[ix] = ratioperleaf[leaf_idx]
                ix += 1
            # print("## Ratio per leaf:",ratioperleaf)
            print("Stats:Mean:{}; Var:{}; leaf_bnds:{}".format( np.mean(ratioperleaf_arr), np.var(ratioperleaf_arr), len(self.leaf_boundary) ))


    def exportpointstodisk(self, exportname, dirname,accuracy_on_testset):
        dir_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), config.resultsdirname, dirname)
        if not os.path.exists(dir_path):
            os.makedirs(dir_path)

        # I. Export points
        X = np.array(self.points)
        # np.savetxt(os.path.join(dir_path, "pointsextraction_" + exportname + ".csv"), X, delimiter="\t", header=" X \t Y", comments="")

        # II. Export ratios
        # XU represents the ratios, I sampled the ratio value given by config.attack_check_intervals
        # In the latter case, do not start at 0, because after the first query, I start to observe.
        # Moreover, add as last row the global query count and its ratio...
        # the last row additionally is more meta data, it contains the #leaves...

        if self.last_query_count_when_detect_attack_called > self.global_query_count:
            raise Exception("XU != intervals Problem")
        # Check that global query count matches with last query count when ratios were computed
        self.detect_attack()

        # Two cases: global_query_count is multiple of check_intervals or not:
        intervals = np.array(
            range(config.attack_check_intervals, self.global_query_count+1, config.attack_check_intervals))
        if self.global_query_count % config.attack_check_intervals != 0:
            intervals = np.append(intervals, [self.global_query_count])

        XU = np.array(self.ratios)

        if len(XU) != len(intervals):
            raise Exception("XU != intervals")

        XU = np.column_stack((intervals,XU)) # stack intervals and ratios column-wise tog.
        ## add the total number of queries in last row
        XU = np.row_stack( (XU, np.array([accuracy_on_testset, len(self.tree.get_leaf_ids())]) ) )
        np.savetxt(os.path.join(dir_path, "ratios_" + exportname + ".dat"), XU, delimiter="\t", header=" X \t Y", comments="")


    def get_last_ratio(self):
        if self.last_query_count_when_detect_attack_called > self.global_query_count:
            raise Exception("XU != intervals Problem")
        # Check that global query count matches with last query count when ratios were computed
        self.detect_attack()

        return self.ratios[-1]



class LeafSecurityMargin:

    def __init__(self, leaf_id, leaf_path, leaf_training_data, feature_boundary_dict):

        # store id that this leaf security margin object represents
        self.lead_id = leaf_id
        # store in directory, for each dimension the ThresholdDimension object
        self.features = {}
        # the number of queries inside this leaf's security margin. If a query lies in two security margins,
        # e.g. because the point lies inside in two dimensions, the query though counts once for this leaf.
        self.counted_unique = 0
        # the total number of queries that have ended in this leaf
        self.counted_queries = 0

        # parse the leaf_path to create the boundary for each dimension that the leaf uses
        for path_tuple in leaf_path:
            cfeature = path_tuple[0]
            cvalue = path_tuple[1]
            csign = path_tuple[2]

            # get the training data values for the particular dimension,
            # via values operator, convert single column to numpy array
            train_data_on_feature = leaf_training_data[cfeature].values

            if cfeature not in self.features:
                self.features[cfeature] = ThresholdDimension(cfeature, train_data_on_feature,
                                                             feature_boundary_dict[cfeature])
                self.features[cfeature].add_threshold(cvalue, csign)
            else:
                self.features[cfeature].add_threshold(cvalue, csign)

        for thrs in self.features:
            self.features[thrs].learn_margin()


    def store_query(self, query):
        insecuritymargin = self.check_if_query_in_margin(query)

        if insecuritymargin:
            self.counted_unique += 1

        self.counted_queries += 1


    def detect_attack(self):
        # unique count, multiple matches in >1 dimensions are counted once
        ratio = 0
        # print "\tUnique count:" + str(self.counted_unique) + str(" to ") + str(self.counted_queries)
        if self.counted_queries > config.MIN_COUNTED_QUERIES_IN_LEAF:
            ratio = self.counted_unique / float(self.counted_queries)

        return ratio


    def check_if_query_in_margin(self, query):
        insecuritymargin = False
        for cfeature in query:
            cvalue = query[cfeature]
            # if feature exists in stored features:
            if cfeature in self.features:
                insecuritymargin = insecuritymargin or self.features[cfeature].check_if_query_in_margin(cvalue)
        return insecuritymargin


class ThresholdDimension:
    """
    Stores the security margin for a particular dimension
    First, use init to initialize the object.
    Then, call add_threshold to learn the two thresholds on the particular dimension based on the path to the leaf
    Finally, call learn_margin to learn the width of the security margin based on the training data

    If threshold is still None after add_threshold, this means that there is no boundary in this direction.
    """

    def __init__(self, feature, train_data, feature_range):
        self.feature = feature
        self.threshold_left = None
        self.threshold_right = None
        self.margin_left = None
        self.margin_right = None
        self.train_data = train_data
        self.feature_range = feature_range
        # self.count_left = 0
        # self.count_right = 0

        # adjust margin
        self.epsilon = 0  # used if min or max of training samples is used to determine boundary of security margin
        self.false_probability = config.false_probability

    def add_threshold(self, threshold, sign):
        """
        Adds the threshold to the object, depending whether it is LE or GE.
        Parameters
        ----------
        threshold
        sign

        Returns
        -------
        Exception if no valid sign
        """
        if sign == "LE":
            if self.threshold_right is None:
                self.threshold_right = threshold
            else:
                # use min of current known and new threshold
                self.threshold_right = min(self.threshold_right, threshold)
        elif sign == "GE":
            if self.threshold_left is None:
                self.threshold_left = threshold
            else:
                self.threshold_left = max(self.threshold_left, threshold)
        else:
            raise AssertionError("No supported sign used")


    def learn_margin(self):
        """
        After initializing with the two thresholds, learn now the security margin based on the training data
        in that region.
        """

        # 0. If threshold is None, set it to max/min range of feature
        if self.threshold_left is None:
            self.threshold_left = self.feature_range[0]
        if self.threshold_right is None:
            self.threshold_right = self.feature_range[1]

        # 1. First option, simply set margin to min/max of training data

        max_margin = np.abs(self.threshold_right - self.threshold_left) * config.margin_rate
        self.margin_left = min(np.abs(self.threshold_left - self.train_data.min()) - self.epsilon, max_margin)
        self.margin_right = min(np.abs(self.threshold_right - self.train_data.max()) - self.epsilon, max_margin)

        # 2. Second option, learn a kernel-density estimate using Gaussian kernels
        k_fold = 5
        # perform kernel density only if at least 5 samples are in the leaf region
        if len(self.train_data) > k_fold:
            # a. learn the optimal bandwidth of Gaussian kernel
            grid = GridSearchCV(KernelDensity(),
                                {'bandwidth': np.linspace(0.01, 1.0, 30)},
                                cv=k_fold)  # k-fold cross-validation
            # fit expects array_like, shape (n_samples, n_features), so convert array to that
            grid.fit(self.train_data[:, np.newaxis])
            kde = grid.best_estimator_
            # print grid.best_params_

            # b. now get threshold where false probability exceeds a well-chosen threshold
            X_area = np.linspace(self.threshold_left, self.threshold_right, 1000)[:, np.newaxis]
            log_dens = kde.score_samples(X_area)
            # now add probabilities until cumulative probability sum is larger than fixed false probability threshold
            # don't ask why we need X_area[:,0] and [0][0] is necessary after np.where
            cum_probabilities = sc.integrate.cumtrapz(np.exp(log_dens), X_area[:,0])

            first_position = np.where(cum_probabilities >= self.false_probability)[0]
            if len(first_position) > 0:
                first_position = X_area[first_position[0]]
            else:
                # probability for training data is still too large on threshold to detect attack reliably
                first_position = self.threshold_left

            last_position = np.where(cum_probabilities >= (1-self.false_probability))[0]
            if len(last_position) > 0:
                last_position = X_area[last_position[0]]
            else:
                # probability for training data is still too large on threshold to detect attack reliably
                last_position = self.threshold_right

            self.margin_left = min(np.abs(self.threshold_left - first_position) - self.epsilon, max_margin)
            self.margin_right = min(np.abs(self.threshold_right - last_position) - self.epsilon, max_margin)


    def check_if_query_in_margin(self, query_value):
        """
        Returns boolean if query value lies inside the security margin
        or if open range if inside or anormally outside of range during training
        """
        # check left security margin
        # use > or < instead of >= and <=, because if no sec margin is there because training samples lie on threshold.
        if self.threshold_left is not None:
            if query_value < self.threshold_left + self.margin_left:
                return True
        if self.threshold_right is not None:
            if query_value > self.threshold_right - self.margin_right:
                return True
        return False

    # def detect_attack(self):
    #     """
    #     Detect attack based on count in security margin's
    #     Returns a tuple
    #     a. with boolean indicator that attack probably happens,
    #     b. count of left security margin,
    #     c. count of right security margin.
    #     -------
    #     """
    #     if self.count_left >= 20 or self.count_right >= 20:
    #         attack_happened = True
    #     else:
    #         attack_happened = False
    #     return attack_happened, self.count_left, self.count_right


# class ThresholdCount:
#
#     def __init__(self, threshold, width):
#         """
#         Inits the Boundary Counter for each decision threshold
#         Parameters
#         ----------
#         threshold - the decision threshold
#         width - the width of the security margin at the decision threshold
#         """
#         self.threshold = threshold
#         self.width = width
#         self.count_in_margin = 0
#
#     def increment(self, val):
#         """
#         Increment threshold counter if val is inside the security margin
#         Parameters
#         ----------
#         val - current query feature value
#         """
#         if val <= self.threshold + self.width and val >= self.threshold - self.width:
#             # lies inside the range
#             self.count_in_margin += 1
#
#     def __str__(self):
#         return str(self.threshold)+"+/-"+str(self.width)+": "+str(self.count_in_margin)
#
#     def __repr__(self):
#         return str(self.threshold)+"+/-"+str(self.width)+": "+str(self.count_in_margin)