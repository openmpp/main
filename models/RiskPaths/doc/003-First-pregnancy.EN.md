### First pregnancy

As outlined above, first pregnancy is modeled by an age baseline hazard and relative risks dependent on union status and duration. The following Table 1 displays the parameter estimates for Bulgaria and Russia before and after the political and economical transition.  

**Table 1: First pregnancy risks**

|table>
        | Bulgaria | Russia 
- | - | -
15-17.5 | 0.2869 | 0.2120 
17.5-20 | 0.7591 | 0.7606 
20-22.5 | 0.8458 | 0.8295 
22.5-25 | 0.8167 | 0.6505 
25-27.5 | 0.6727 | 0.5423 
27.5-30 | 0.5105 | 0.5787 
30-32.5 | 0.4882 | 0.4884 
32.5-35 | 0.2562 | 0.3237 
35-37.5 | 0.2597 | 0.3089 
37.5-40 | 0.1542 | 0.0909 
|<table

|table>
                              | before 1989 | transition | 10 years after | transition: 1999+
                              | Bulgaria    | Russia | Bulgaria           | Russia 
- | - | -
Not in union                  | 0.0648 | 0.0893 | 0.0316 | 0.0664 
First 3 years of first union  | 1.0000 | 1.0000 | 0.4890 | 0.5067 
First union after three years | 0.2523 | 0.2767 | 0.2652 | 0.2746 
Second union                  | 0.8048 | 0.5250 | 0.2285 | 0.2698 
|<table

The data from Table 1 is interpreted as follows in the model. As long as a woman has not entered a partnership, we have to multiply her age-dependent baseline risk of first pregnancy by the relative risk “not in a union”. For example, the pregnancy risk of a 20 year old single woman of the pre-transition Bulgarian cohort can be calculated as `0.8458 * 0.0648 = 0.05481`. At this rate of `λ = 0.05481`: 

- The expected mean waiting time to the pregnancy event is `1/λ = 1/0.05481 = 18.25` years; 
- The probability that a women does not experience pregnancy in the following 2.5 years (given that she stays single) is `exp(-λt) = exp(-0.05481*2.5) = 87.2%`. 

Thus at her 20th birthday, we can draw a random duration to first pregnancy from a uniform distributed random number (a number that can obtain any value between 0 and 1 with the same probability) using the formula: 

`RandomDuration = -ln(RandomUniform) / λ`; 

As we have calculated above, in 87.2% of the cases, no conception will take place in the next 2.5 years. Accordingly, if we draw a uniform distributed random number smaller than 0.872, the 
corresponding waiting time will be longer than 2.5 years, since  
`-ln(RandomUniform) / λ = -ln(0.872)/0.05481 = 2.5` years. A random draw greater than 0.872 will result in a waiting time smaller than 2.5 years–in this situation, if the woman does not enter a union before the pregnancy event, the pregnancy takes place in our simulation.  

To continue this example, let us assume that the first event that happens in our simulation is a union formation at age 20.5. We now have to update the pregnancy risk. While the baseline risk still stays the same for the next two years (i.e. 0.8458), the relative risk is now 1.0000 (as per the reference category in Table 1) because the woman is in the first three years of a union. The new hazard rate for pregnancy (applicable for the next two years, until age 22.5) is considerably higher now at `0.8458 * 1.0000 = 0.8458`. The average waiting time at this rate is thus only `1/0.8458 = 1.18` years and for any random number greater than `exp(-0.8458*2)=0.1842` the simulated waiting time would be smaller than two years. That is, 81.6% (`1 – 0.1842`) of women will experience a first pregnancy within the first two years of a first union or partnership that begins at age 20.5.  


[[Home](#Home)] [[<< Prev](#002-General-description)] [[Next >>](#004-First-union-formation)]