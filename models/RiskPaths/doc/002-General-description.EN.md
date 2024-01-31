### General Descripotion

Being a model for the study of childlessness, the main event of RiskPaths is the first pregnancy (which is always assumed to lead to birth). Pregnancy can occur at any point in time after the 15th birthday, with risks changing by both age and union status. The underlying statistical models are piecewise constant hazard regressions. With respect to fertility this implies the assumption of a constant pregnancy risk for a given age group (e.g. age 15-17.5) and union status (e.g. single with no prior unions).  
For unions, we distinguish four possible state levels:  
-	single  
-	the first three years in a first union  
-	the following years in a first union  
-	all the years in a second union  
(After the dissolution of a second union, women are assumed to stay single). Accordingly, we model five different union events:  
-	first union formation 
-	first union dissolution 
-	second union formation 
-	second union dissolution 
-	the change of union phase which occurs after three years in the first union.  
The last event (change of union phase) is a clock event – it differs from other events in that its timing is not stochastic but predefined. (Another clock event in the model is the change of the age index every 2.5 years) Besides unions and fertility, we model mortality--a woman may die at any point in time. We stop the simulation of the pregnancy and union events either when a women dies, or at pregnancy (as we are only interested in studying childlessness), or at her 40th birthday (since later first pregnancies are very rare in Russia and Bulgaria and are thus ignored for this model).  
At age fifteen a woman becomes subject to both pregnancy and union formation risks. These are competing risks. We draw random durations to first pregnancy and to first union formation. There are two additional competing events at this stage—mortality and change of age group. (As we assume that both pregnancy and union formation risks change with age, the risks underlying the random durations only apply for a given time period--2.5 years in our model--and have to be recalculated at that point in time.)  
In other words, the 15th birthday will be followed by one of these four possible events:  
-	the woman dies 
-	she gets pregnant  
-	she enters a union 
-	she enters a new age group at age 17.5 because none of the first three events occurred before age 17.5 
Death or pregnancy terminates the simulation. A change of age index requires that the waiting times for the competing events union formation and pregnancy be updated. The union formation event alters the risk of first pregnancy (making it much higher) and changes the set of competing risks. A woman is then no longer at risk of first union formation but becomes subject to union dissolution risk.  


[[Home](#Home)] [[<< Previous](#Introduction)] [[Next >>](#QQQ)]