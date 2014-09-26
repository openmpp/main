--
-- Copyright (c) 2014 OpenM++
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
SELECT S.dim0, S.dim1, S.value
FROM modelone_201208171604590148_p0_ageSex S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);
GO

CREATE VIEW salaryAge 
AS 
SELECT S.dim0, S.dim1, S.value
FROM modelone_201208171604590148_p1_salaryAge S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);
GO

--
-- Output tables compatibility views
--
CREATE VIEW salarySex 
AS 
SELECT
  S.dim0    AS "Dim0",
  S.dim1    AS "Dim1",
  S.unit_id AS "Dim2",
  S.value   AS "Value"
FROM modelone_201208171604590148_v0_salarySex S
WHERE S.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = 1);
GO
