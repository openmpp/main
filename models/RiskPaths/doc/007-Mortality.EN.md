### Mortality 

In this sample model, we leave it to the model user to either set death probabilities by age or to “switch off” mortality allowing the study of fertility without interference from mortality. In the latter case, all women reach the maximum age of 100 years. If the user chooses to simulate mortality, the specified probabilities are internally converted to piecewise constant hazard rates (based on the formula -ln(1-p) for p<1) so that death can happen at any time in a year. If a probability is set to 1 (as is the case when age=100), immediate death is assumed.


[[Home](#Home)] [[<< Prev](#006-Union-dissolution)] [[Next >>](#008-QQQ)]