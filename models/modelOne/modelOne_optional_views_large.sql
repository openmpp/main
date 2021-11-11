--
-- Copyright (c) 2021 OpenM++
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
CREATE VIEW salaryByYears 
AS 
SELECT S.param_value AS "Value"
FROM salaryByYears_p_2012818 S
WHERE S.sub_id = 0
AND S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW salaryByPeriod 
AS 
SELECT S.param_value AS "Value"
FROM salaryByPeriod_p_2012819 S
WHERE S.sub_id = 0
AND S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW salaryByLow 
AS 
SELECT S.param_value AS "Value"
FROM salaryByLow_p_201281_10 S
WHERE S.sub_id = 0
AND S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW salaryByMiddle 
AS 
SELECT S.param_value AS "Value"
FROM salaryByMiddle_p_201281_11 S
WHERE S.sub_id = 0
AND S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

--
-- Output tables compatibility views
--
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
