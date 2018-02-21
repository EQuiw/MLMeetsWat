from abc import ABCMeta, abstractmethod

class Countermeasure:
    __metaclass__ = ABCMeta

    def __init__(self, reaction):
        self.detected_attack = False
        self.reaction = reaction

        # counts all queries made so far
        self.global_query_count = 0
        self.last_query_count_when_detect_attack_called = 0

    @abstractmethod
    def set_tree(self, tree):
        """
        Sets the reference to the tree object to which this object belongs
        :param tree:
        """
        pass

    @abstractmethod
    def store_query(self, query, idx=None):
        """
        Stores the current query made by an user
        :param query: query is probably a dictionary where the features are keys
        :param idx: the leaf id that was reached in learned tree
        :return: nothing
        """
        pass

    @abstractmethod
    def detect_attack(self):
        """
        Evaluates the queries made so far and decides if an attack is occuring
        :return: true if queries made so far are attack queries, false if legitimate queries
        """
        pass

    @abstractmethod
    def let_countermeasure_decide(self, cleaf_id):
        """
        Returns what the tree should do based on the countermeasure's information
        :return: the correct or wrong leaf_id, or an BlockedException.
        """
        pass

    @abstractmethod
    def check_if_query_is_malicious(self, query, idx):
        """
        Checks whether the countermeasure thinks the current query seems wrong.
        In contrast to store_query, this method does not cause any changes in
        the internal state of the countermeasure.
        :param query: query is probably a dictionary where the features are keys
        :param idx: the leaf id that was reached in learned tree
        :return:
        """

    @abstractmethod
    def pprint(self):
        """
        Pretty print of object
        :return:
        """
        pass


    @abstractmethod
    def exportpointstodisk(self, exportname, dirname,accuracy_on_testset):
        """
        Exports the query points (stored in self.points) to the disk as table -- so that are usable e.g. in pgfplots
        """
        pass

    @abstractmethod
    def get_last_ratio(self):
        """
        Returns the last observed security ratio. Useful to get the ratio after finishing the attack quickly.
        """
        pass
