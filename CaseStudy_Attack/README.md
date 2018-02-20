# Case Study: Deep Learning against Digital Watermarking
We apply concepts from adversarial learning---actually examined to find adversarial examples
against Deep Neural Networks---to attack a watermarking scheme.


# What you need
- R and I recommend RStudio 
- My R package ForgottenSiblings.
    - You may have to adapt the Makevars file in the src directory. Maybe you have to remove the
        fPIC option or to provide the correct paths.
- gnu parallels (only if you use the bash script 1_cropImages.sh)
- Enough memory, if you use images larger than 96x96 px, then have at least 32 GB.
It's R. If you have simple ideas to handle the images more efficiently, contact me ;)


# Overview / Steps

- In Rscripts, you find the scripts for the different steps of the attack procedure.
- the ForgottenSiblings directory represents the R package that you will need.
- The ManuallySelectedImageSet directory contains some images that we have manually selected
from popular images like Lena and have meaningful content despite the cropping process. 
We use it as additional test set. You can create multiple smaller images by selecting
various offsets.

## Rscripts
1. First, you need the data. Use 1_cropImages.sh
2. Use 2_getdata.R to generate the watermarked and unmarked images and to save
this set as RData
3. Use 3_attack.R to learn the model and to conduct an attack. Feel free
to first try different architectures, and then to conduct the attack.


If you have any questions, do not hesitate to contact me and I will try to clarify 
the parts that need more documentation. We are currently extending this case 
study for future work so that the code may change in future. If I find some time,
I will extend the documentation or rewrite code parts that were quickly added during 
the last experiments ;)
