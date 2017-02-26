--
-- Copyright (c) 2017 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--

--
-- Compatibility views
--
-- Dear user:
-- this part of database is optional and NOT used by openM++
-- if you want to have it for any reason please enjoy
-- if you don't need it then ignore everything below
--

--
-- Input parameters compatibility views
--
CREATE VIEW ageSex 
AS 
SELECT S.dim0, S.dim1, S.param_value AS "Value"
FROM ageSex_p_2012817 S
WHERE S.sub_id = 0
AND S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW salaryAge 
AS 
SELECT S.dim0, S.dim1, S.param_value AS "Value"
FROM salaryAge_p_2012818 S
WHERE S.sub_id = 0
AND S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW StartingSeed 
AS 
SELECT S.param_value AS "Value"
FROM StartingSeed_p_2012819 S
WHERE S.sub_id = 0
AND S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW salaryFull 
AS 
SELECT S.dim0, S.param_value AS "Value"
FROM salaryFull_p_2012812 S
WHERE S.sub_id = 0
AND S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW baseSalary 
AS 
SELECT S.param_value AS "Value"
FROM baseSalary_p_2012811 S
WHERE S.sub_id = 0
AND S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW filePath 
AS 
SELECT S.param_value AS "Value"
FROM filePath_p_2012814 S
WHERE S.sub_id = 0
AND S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

--
-- Output tables compatibility views
--
CREATE VIEW salarySex 
AS 
SELECT
  S.dim0       AS "Dim0",
  S.dim1       AS "Dim1",
  S.expr_id    AS "Dim2",
  S.expr_value AS "Value"
FROM salarySex_v_2012820 S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);
