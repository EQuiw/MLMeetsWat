from tree_stealer import *
import numpy as np

class TreeScikitExtractor(TreeExtractor):
    """
    Algorithm for extracting a decision tree in Scikitlearn from black-box queries.
    The attacker puts some additional fake queries to each benign query. The values are randomly created
    in the range of the possible features.
    """

    def __init__(self, decisiontree, epsilon=0.01, rounding=None, addnoise=0):
        self.decisiontree = decisiontree
        self.addnoiseagainstcountermeasure = addnoise
        self.noisyqueryCount = 0
        self.noisyqueriesinmargin = 0
        self.attqueriesinmargin = 0

        super(TreeScikitExtractor, self).__init__(epsilon=epsilon, rounding=rounding)


    #@Overwrite
    def get_features(self):
        return self.decisiontree.get_features()


    #@Overwrite
    def make_prediction(self, query):
        ret = self.decisiontree.make_prediction(query)
        if self.decisiontree.countermeasure.check_if_query_is_malicious(query, ret):
            self.attqueriesinmargin += 1

        # Send addnoiseagainstcountermeasure fake queries for each normal query to hide attack traces
        if self.addnoiseagainstcountermeasure > 0:
            for n_q in range(0,self.addnoiseagainstcountermeasure):
                noisyquery = copy(query)

                for feat in self.features:
                    if isinstance(feat, ContFeature):
                        noisyquery[feat.name] = np.random.uniform( feat.min_val , feat.max_val )
                    else:
                        raise Exception("I guess I do not work with non-continous features currently")

                self.noisyqueryCount += 1
                res_id2 = self.decisiontree.make_prediction(noisyquery)
                if self.decisiontree.countermeasure.check_if_query_is_malicious(noisyquery, res_id2):
                    self.noisyqueriesinmargin += 1

        return ret

