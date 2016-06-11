--
-- Copyright (c) 2013 OpenM++
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
FROM ageSex_p2012_817 S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

CREATE VIEW salaryAge 
AS 
SELECT S.dim0, S.dim1, S.param_value AS "Value"
FROM salaryAge_p2012_818 S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);

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
FROM salarySex_v2012_820 S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);
