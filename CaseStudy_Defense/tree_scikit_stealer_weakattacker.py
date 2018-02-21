from tree_scikit_stealer import *
import numpy as np
from sklearn.grid_search import GridSearchCV
from sklearn.neighbors import KernelDensity
import config_countermeasure as config

class TreeScikitWeakAttackerExtractor(TreeScikitExtractor):
    """
    Algorithm for extracting a decision tree in Scikitlearn from black-box queries.
    The attacker puts some additional fake queries to each benign query based on some known train samples.
    """

    def __init__(self, decisiontree, x_train, epsilon=0.01, rounding=None, addnoise=0):
        self.x_train = x_train
        super(TreeScikitWeakAttackerExtractor, self).__init__(decisiontree=decisiontree, epsilon=epsilon,
                                                              rounding=rounding, addnoise=addnoise)
        # self.kde = None # the best kernel density estimator
        self.sam = None
        # self.learn_train_distribution()

    #@Overwrite
    def make_prediction(self, query):
        ret = self.decisiontree.make_prediction(query)
        if self.decisiontree.countermeasure.check_if_query_is_malicious(query, ret):
            self.attqueriesinmargin += 1

        # Send addnoiseagainstcountermeasure fake queries for each normal query to hide attack traces
        if self.addnoiseagainstcountermeasure > 0:
            for n_q in range(0,self.addnoiseagainstcountermeasure):
                noisyquery = copy(query)
                next_query = self.__get_next_plausible_random_point()

                i = 0
                for feat in self.features:
                    if isinstance(feat, ContFeature):
                        noisyquery[feat.name] = next_query[i]
                        i += 1
                    else:
                        raise Exception("I guess I do not work with non-continous features currently")

                # Hide the traces
                self.noisyqueryCount += 1
                res_id2 = self.decisiontree.make_prediction(noisyquery)
                # Evaluation: how much falls in margin
                if self.decisiontree.countermeasure.check_if_query_is_malicious(noisyquery, res_id2):
                    self.noisyqueriesinmargin += 1

        return ret


    # def learn_train_distribution(self):
    #     """
    #     Based on the known training samples, this method learns the data distribution
    #     so that attacker can guess a more plausible query point as fake query
    #     """
    #     grid = GridSearchCV(KernelDensity(),
    #                         {'bandwidth': np.linspace(0.01, 1.0, 30)},
    #                         cv=3)  # k-fold cross-validation
    #     grid.fit(self.x_train)
    #
    #     # We employ the best estimator from CV
    #     self.kde = grid.best_estimator_
    #     self.sam = self.kde.sample(1, random_state=41)[0]


    def __get_next_plausible_random_point(self):
        """
        Returns a plausible fake query depending on the known training set
        :return: fake query
        """
        # we simply sample from distribution
        # sampled = self.kde.sample(1, random_state=self.queryCount)[0]

        if config.usealltraindata:
            # Either select from known train data randomly one
            number_trainsamples = self.x_train.shape[0]
            return np.array(self.x_train.iloc[np.random.randint(0,number_trainsamples),:])
        else:
            # or select one train data, guaranteed not in margin (we giving att. this knowledge), always
            if self.sam is None:
                for i in range(0, self.x_train.shape[0]):
                    crow = np.array(self.x_train.iloc[i, :])
                    ix = 0
                    query = {}
                    for feat in self.features:
                        if isinstance(feat, ContFeature):
                            query[feat.name] = crow[ix]
                            ix += 1
                        else:
                            raise RuntimeError()
                    res_id2 = self.decisiontree.make_prediction(query)
                    # Evaluation: how much falls in margin
                    if not self.decisiontree.countermeasure.check_if_query_is_malicious(query, res_id2):
                        self.sam = crow
                        print("The leaf id of always same fake query is:{}".format(res_id2))
                        return self.sam
                raise Exception("No training sample is outside the margin. Really? What's wrong with you?")

            return self.sam