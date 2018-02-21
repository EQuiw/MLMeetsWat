from sklearn import tree
import pandas as pd
import numpy as np
from trees.feature import *

import config_countermeasure as config


class DecisionTreeScikit:
    """
    The decision tree. Try various hyper parameters, thus adjust the decision tree learning in the init method.
    """

    def __init__(self, x_train, y_train):
        """
        Constructor
        :param x_train: the training data for Scikit Tree
        :param y_train: labels of training data
        :param countermeasure: A Countermeasure object that can be used against model stealing attacks
        """

        self.x_train = x_train
        self.y_train = y_train
        self.countermeasure = None

        # create feature list, learn feature range on whole dataset
        self.features = []
        key = 0
        for colno in range(0, len(self.x_train.columns)):
            col = self.x_train.columns[colno]
            f = ContFeature(col, key,
                            min(x_train.iloc[:, colno].min(), config.MINFEATURE),
                            max(x_train.iloc[:, colno].max(), config.MAXFEATURE))
            self.features.append(f)
            key += 1

        # learn the decision tree that will be extracted by attack
        self.clf = tree.DecisionTreeClassifier(random_state=42, min_samples_leaf=4,class_weight="balanced")
        self.clf.fit(self.x_train, self.y_train)

        # get more information about itself: the structure of the tree, necessary for countermeasures
        self.idx = None
        # save later the unique feature path of each child node
        self.idx_path = {}
        # store for each feature the thresholds, it is a dictionary of features. Element is a list of thresholds.
        self.feature_thresholds = {}
        self.get_lineage()  # fills idx and feature_thresholds

        # last predict stores the class name of the last predicted element
        self.last_predict = None


    def set_countermeasure_strategy(self, countermeasure):
        self.countermeasure = countermeasure
        self.countermeasure.set_tree(self)


    def get_leaf_ids_from_training_data(self):
        # get leaf id's for training data, needed for distribution countermeasure
        X_train = np.array(self.x_train)
        X_train = X_train.astype('float32')
        leaf_ids = self.clf.tree_.apply(X_train)
        return leaf_ids


    def get_features(self):
        return self.features


    def make_prediction(self, query):
        """
        Makes a prediction for the user. It also uses a countermeasure implementation to detect attacks.
        Once an attack was detected by the countermeasure, this tree can take actions for future queries.
        :param query:
        :return:
        """
        # Here we use a real unique leaf id, whereas BigML and the evaluation in the Usenix paper assumed that
        # the leaf id is built from a confidence score, class name, list of unique used features..
        # We thus evaluate under a stronger attacker..

        # What is done here:
        # We get the query as dictionary, form an respective numpy array to get the leaf id from scikit learn
        querx = []
        for f in self.features:
            fname = f.name
            queryval = query[fname]
            querx.append(queryval)

        # Now get the lead id for current query from learned tree in scikit learn
        querx = np.array(querx, dtype="float32")
        querx = querx.reshape((1,len(self.features)))
        # apply expects two-dim array
        leaf_id = int(self.clf.tree_.apply(querx))
        self.last_predict = self.clf.predict(querx)

        # if countermeasure was given initially:
        if self.countermeasure is not None:
            # send query to countermeasure
            self.countermeasure.store_query(query, leaf_id)
            # let countermeasure decide what output should be returned. Either correct leaf_id (no attack),
            # or random leaf id or Exception (if further access should be blocked).
            leaf_id = self.countermeasure.let_countermeasure_decide(leaf_id)

        return leaf_id


    def get_leaf_ids(self):
        return self.idx

    def get_decision_thresholds(self):
        return self.feature_thresholds

    def get_lineage(self):
        """
        Extracts the decision rules from the learned tree.
        Taken from
        http://stackoverflow.com/questions/20224526/how-to-extract-the-decision-rules-from-scikit-learn-decision-tree
        Next, it stores for each feature/dimension the original thresholds
        """
        tree = self.clf
        feature_names = [f.name for f in self.features]

        left = tree.tree_.children_left
        right = tree.tree_.children_right
        threshold = tree.tree_.threshold
        features = [feature_names[i] for i in tree.tree_.feature]

        # get ids of child nodes
        self.idx = np.argwhere(left == -1)[:, 0]
        for ixs in self.idx:
            self.idx_path[ixs] = []

        for ff in feature_names:
            self.feature_thresholds[ff] = []

        def recurse(left, right, child, lineage=None):
            if lineage is None:
                lineage = [child]
            if child in left:
                parent = np.where(left == child)[0].item()
                split = 'l'
            else:
                parent = np.where(right == child)[0].item()
                split = 'r'

            lineage.append((parent, split, threshold[parent], features[parent]))

            if parent == 0:
                lineage.reverse()
                return lineage
            else:
                return recurse(left, right, parent, lineage)

        for child in self.idx:
            for node in recurse(left, right, child):
                # print node
                # prints the following... e.g.
                # (0, 'l', 0.80000001192092896, 'petal width')
                # (0, 'r', 0.80000001192092896, 'petal width')

                if isinstance(node, tuple):
                    # a. just add if tuple = an internal node, no leaf
                    if node[2] not in self.feature_thresholds[node[3]]:
                        # see above, an internal node is printed twice, left and right
                        self.feature_thresholds[node[3]].append(node[2])

                    # b. add to path of current child idx
                    x = "LE" if node[1] == "l" else "GE"
                    self.idx_path[child].append((node[3],node[2],x))


        # print("Feature Thresholds:")
        # print(self.feature_thresholds)
        # print(self.idx_path)