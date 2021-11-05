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

CREATE VIEW isOldAge 
AS 
SELECT S.dim0, S.param_value AS "Value"
FROM isOldAge_p_2012815 S
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
FROM salarySex_v_2012882 S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW fullAgeSalary 
AS 
SELECT
  S.dim0       AS "Dim0",
  S.dim1       AS "Dim1",
  S.expr_id    AS "Dim2",
  S.dim2       AS "Dim3",
  S.expr_value AS "Value"
FROM fullAgeSalary_v_2012883 S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW ageSexIncome 
AS 
SELECT
  S.dim0       AS "Dim0",
  S.dim1       AS "Dim1",
  S.expr_value AS "Value"
FROM ageSexIncome_v_2012884 S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW seedOldAge 
AS 
SELECT
  S.expr_value AS "Value"
FROM seedOldAge_v_2012885 S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW incomeByYear 
AS 
SELECT
  S.dim0       AS "Dim0",
  S.dim1       AS "Dim1",
  S.dim2       AS "Dim2",
  S.dim3       AS "Dim3",
  S.expr_value AS "Value"
FROM incomeByYear_v_2012105 S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW incomeByLow 
AS 
SELECT
  S.dim0       AS "Dim0",
  S.dim1       AS "Dim1",
  S.dim2       AS "Dim2",
  S.dim3       AS "Dim3",
  S.expr_value AS "Value"
FROM incomeByLow_v_2012106 S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW incomeByMiddle 
AS 
SELECT
  S.dim0       AS "Dim0",
  S.dim1       AS "Dim1",
  S.dim2       AS "Dim2",
  S.dim3       AS "Dim3",
  S.expr_value AS "Value"
FROM incomeByMiddle_v_2012107 S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW incomeByPeriod 
AS 
SELECT
  S.dim0       AS "Dim0",
  S.dim1       AS "Dim1",
  S.dim2       AS "Dim2",
  S.dim3       AS "Dim3",
  S.expr_value AS "Value"
FROM incomeByPeriod_v_2012108 S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);
