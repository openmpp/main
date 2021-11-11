--
-- Copyright (c) 2021 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--

--
-- list of ids: values < 10 reserved for development and testing
--   model id = 1 is modelOne
--   set id = 2 is default set of input parameters for modelOne
--   set id = 4 modified set of input parameters
--

--
-- modelOne default set of parameters: must include ALL model parameters
--
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 108, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 109, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 110, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 111, 1, 0);

INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 108, 0, 'Salary by Years default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 109, 0, 'Salary by Period default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 110, 0, 'Salary by Low Period default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 111, 0, 'Salary by Middle Period default values');

--
-- modelOne other set of parameters
-- it is not based on model run and does include ALL model parameters
--
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 108, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 109, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 110, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 111, 1, 0);

--
-- modelOne input parameters
--

--
-- set id = 2
--

-- Salary by Years default values
INSERT INTO salaryByYears_w_2012818 (set_id, sub_id, dim0, dim1, dim2, dim3, param_value)
SELECT
  2, 0, AG.enum_id, SX.enum_id, SL.enum_id, YR.enum_id, (AG.enum_id + SX.enum_id + SL.enum_id + YR.enum_id)
FROM type_enum_lst AG, type_enum_lst SX, type_enum_lst SL, type_enum_lst YR
WHERE AG.type_hid = 101 AND SX.type_hid = 102 AND SL.type_hid = 103 AND YR.type_hid = 105;

-- Salary by Period default values
INSERT INTO salaryByPeriod_w_2012819 (set_id, sub_id, dim0, dim1, dim2, dim3, dim4, param_value)
SELECT
  2, 0, AG.enum_id, SX.enum_id, SL.enum_id, YR.enum_id, PD.enum_id, (AG.enum_id + SX.enum_id + SL.enum_id + YR.enum_id + PD.enum_id)
FROM type_enum_lst AG, type_enum_lst SX, type_enum_lst SL, type_enum_lst YR, type_enum_lst PD
WHERE AG.type_hid = 101 AND SX.type_hid = 102 AND SL.type_hid = 103 AND YR.type_hid = 105 AND PD.type_hid = 106;

-- Salary by Low Period default values
INSERT INTO salaryByLow_w_201281_10 (set_id, sub_id, dim0, dim1, dim2, dim3, dim4, param_value)
SELECT
  2, 0, AG.enum_id, SX.enum_id, SL.enum_id, YR.enum_id, LP.enum_id, (AG.enum_id + SX.enum_id + SL.enum_id + YR.enum_id + LP.enum_id)
FROM type_enum_lst AG, type_enum_lst SX, type_enum_lst SL, type_enum_lst YR, type_enum_lst LP
WHERE AG.type_hid = 101 AND SX.type_hid = 102 AND SL.type_hid = 103 AND YR.type_hid = 105 AND LP.type_hid = 107;

-- Salary by Middle Period default values
INSERT INTO salaryByMiddle_w_201281_11 (set_id, sub_id, dim0, dim1, dim2, dim3, dim4, param_value)
SELECT
  2, 0, AG.enum_id, SX.enum_id, SL.enum_id, YR.enum_id, MP.enum_id, (AG.enum_id + SX.enum_id + SL.enum_id + YR.enum_id + MP.enum_id)
FROM type_enum_lst AG, type_enum_lst SX, type_enum_lst SL, type_enum_lst YR, type_enum_lst MP
WHERE AG.type_hid = 101 AND SX.type_hid = 102 AND SL.type_hid = 103 AND YR.type_hid = 105 AND MP.type_hid = 108;


--
-- set id = 4
--

-- Salary by Years default values
INSERT INTO salaryByYears_w_2012818 (set_id, sub_id, dim0, dim1, dim2, dim3, param_value)
SELECT
  4, sub_id, dim0, dim1, dim2, dim3, 4 + param_value
FROM salaryByYears_w_2012818
WHERE set_id = 2 AND sub_id = 0;

-- Salary by Period default values
INSERT INTO salaryByPeriod_w_2012819 (set_id, sub_id, dim0, dim1, dim2, dim3, dim4, param_value)
SELECT
  4, sub_id, dim0, dim1, dim2, dim3, dim4, 5 + param_value
FROM salaryByPeriod_w_2012819
WHERE set_id = 2 AND sub_id = 0;

-- Salary by Low Period default values
INSERT INTO salaryByLow_w_201281_10 (set_id, sub_id, dim0, dim1, dim2, dim3, dim4, param_value)
SELECT
  4, sub_id, dim0, dim1, dim2, dim3, dim4, 6 + param_value
FROM salaryByLow_w_201281_10
WHERE set_id = 2 AND sub_id = 0;

-- Salary by Middle Period default values
INSERT INTO salaryByMiddle_w_201281_11 (set_id, sub_id, dim0, dim1, dim2, dim3, dim4, param_value)
SELECT
  4, sub_id, dim0, dim1, dim2, dim3, dim4, 7 + param_value
FROM salaryByMiddle_w_201281_11
WHERE set_id = 2 AND sub_id = 0;
