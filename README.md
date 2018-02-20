# MLMeetsWat
Related to the paper: *Forgotten Siblings: Unifying Attacks on Machine Learning and Digital Watermarking*,
published at the *3rd IEEE European Symposium on Security and Privacy (Euro S&P 2018)*.

## Description
Machine learning is increasingly used in security-critical applications, such as autonomous driving, face recognition and malware detection. Most learning methods, however, have not been designed with security in mind and thus are vulnerable to different types of attacks. This problem has motivated the research field of adversarial machine learning that is concerned with attacking and defending learning methods. Concurrently, a separate line of research has tackled a very similar problem: In digital watermarking, a pattern is embedded in a signal in the presence of an adversary. As a consequence, this research field has also extensively studied techniques for attacking and defending watermarking methods.

The two research communities have worked in parallel so far, unnoticeably developing similar attack and defense strategies. This paper is a first effort to bring these communities together. To this end, we present a unified notation of black-box attacks against machine learning and watermarking. To demonstrate its efficacy, we apply concepts from watermarking to machine learning and vice versa. We show that countermeasures from watermarking can mitigate recent model-extraction attacks and, similarly, that techniques for hardening machine learning can fend off oracle attacks against watermarks. We further demonstrate a novel threat for watermarking schemes based on recent deep learning attacks from adversarial learning. Our work provides a conceptual link between two research fields and thereby opens novel directions for improving the security of both, machine learning and digital watermarking.

<p align="center">
<img src="./intro-watmeetsml.jpg" width="450" height="199" alt="Example of similar attacks in DW and ML" />
</p>

You can find a copy of our paper [here](https://www.tu-braunschweig.de/Medien-DB/sec-team/erwin/mlmeetswat_eurosp2018.pdf).

## Structure

You find the code and datasets for two case studies in this repository.

1. In *CaseStudy_Attack*, you find our first case study where
we transfered concepts from adversarial learning to attack the watermarking
scheme Broken Arrows.

2. In *CaseStudy_Defense*, you find our third case study
where we transfered the closeness-to-the-boundary concept to mitigate model-extraction
attacks on decision trees.

In each directory, you will find another README with more details. 
If you have any questions, just contact me.

If you use the code, please cite the Euro SP paper. 

