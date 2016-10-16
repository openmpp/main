--
-- Copyright (c) 2013 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--

--
-- list of ids: values < 10 reserved for development and testing
--   model id = 1 is modelOne
--   set id = 2 is default set of input parameters for modelOne
--   set id = 3, 4 modified working sets of input parameters
--

-- 
-- modelOne default profile: default run options
--
INSERT INTO profile_lst (profile_name) VALUES ('modelOne');

INSERT INTO profile_option (profile_name, option_key, option_value) VALUES ('modelOne', 'Parameter.StartingSeed', '1023');
INSERT INTO profile_option (profile_name, option_key, option_value) VALUES ('modelOne', 'OpenM.SparseOutput', 'true');
-- INSERT INTO profile_option (profile_name, option_key, option_value) VALUES ('modelOne', 'OpenM.SparseNullValue', '');
-- INSERT INTO profile_option (profile_name, option_key, option_value) VALUES ('modelOne', 'General.Subsamples', '1');

--
-- modelOne default set of parameters: must include ALL model parameters
--
INSERT INTO workset_lst 
  (set_id, base_run_id, model_id, set_name, is_readonly, update_dt) 
VALUES 
  (2, NULL, 1, 'modelOne', 1, '2013-05-29 23:55:07.1234');

INSERT INTO workset_txt(set_id, lang_id, descr, note) VALUES (2, 0, 'Model One default set of parameters', NULL);
INSERT INTO workset_txt(set_id, lang_id, descr, note) VALUES (2, 1, '(FR) Model One default set of parameters', NULL);

INSERT INTO workset_parameter (set_id, parameter_hid) VALUES (2, 4);
INSERT INTO workset_parameter (set_id, parameter_hid) VALUES (2, 5);
INSERT INTO workset_parameter (set_id, parameter_hid) VALUES (2, 6);
INSERT INTO workset_parameter (set_id, parameter_hid) VALUES (2, 7);

INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 4, 0, 'Age by Sex default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 4, 1, '(FR) Age by Sex default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 5, 0, 'Salary by Age default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 5, 1, '(FR) Salary by Age default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 6, 0, 'Starting seed default value');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 7, 0, 'Full or part time by Salary default values');

--
-- modelOne modified set of parameters
-- use this workset for test only: it is INVALID set 
-- workset must either contain all parameters or have base run_id not null
--
INSERT INTO workset_lst 
  (set_id, base_run_id, model_id, set_name, is_readonly, update_dt) 
VALUES 
  (3, NULL, 1, 'modelOne_set', 0, '2013-05-30 23:55:07.1234');

INSERT INTO workset_txt(set_id, lang_id, descr, note) VALUES (3, 0, 'modelOne modified set of parameters', NULL);

INSERT INTO workset_parameter (set_id, parameter_hid) VALUES (3, 4);

INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (3, 4, 0, 'Age by Sex modified values');

--
-- modelOne other set of parameters
-- it is not based on model run and therefore must include ALL model parameters
--
INSERT INTO workset_lst 
  (set_id, base_run_id, model_id, set_name, is_readonly, update_dt) 
VALUES 
  (4, NULL, 1, 'modelOne_other', 1, '2013-05-29 23:55:07.1234');

INSERT INTO workset_txt(set_id, lang_id, descr, note) VALUES (4, 0, 'Model One other set of parameters', NULL);
INSERT INTO workset_txt(set_id, lang_id, descr, note) VALUES (4, 1, '(FR) Model One other set of parameters', NULL);

INSERT INTO workset_parameter (set_id, parameter_hid) VALUES (4, 4);
INSERT INTO workset_parameter (set_id, parameter_hid) VALUES (4, 5);
INSERT INTO workset_parameter (set_id, parameter_hid) VALUES (4, 6);
INSERT INTO workset_parameter (set_id, parameter_hid) VALUES (4, 7);

--
-- modelOne modelling tasks
--
INSERT INTO task_lst (task_id, model_id, task_name) VALUES (1, 1, 'taskOne');

INSERT INTO task_txt (task_id, lang_id, descr, note) VALUES (1, 0, 'Task One for Model One', 'Task One: two set of input parameters');
INSERT INTO task_txt (task_id, lang_id, descr, note) VALUES (1, 1, '(FR) Task One for Model One', NULL);

INSERT INTO task_set (task_id, set_id) VALUES (1, 2);
INSERT INTO task_set (task_id, set_id) VALUES (1, 4);

--
-- modelOne input parameters
--

-- age by sex default values, set id = 2
-- enum ids for age not a zero based
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (2, 10, 0, 0.1);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (2, 10, 1, 0.2);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (2, 20, 0, 0.3);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (2, 20, 1, 0.4);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (2, 30, 0, 0.5);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (2, 30, 1, 0.6);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (2, 40, 0, 0.7);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (2, 40, 1, 0.8);

-- salary by age default values, set id = 2
-- enum ids for age and salary not a zero based
INSERT INTO salaryAge_w_2012818 (set_id, dim0, dim1, param_value) VALUES (2, 100, 10, 10);
INSERT INTO salaryAge_w_2012818 (set_id, dim0, dim1, param_value) VALUES (2, 100, 20, 20);
INSERT INTO salaryAge_w_2012818 (set_id, dim0, dim1, param_value) VALUES (2, 100, 30, 30);
INSERT INTO salaryAge_w_2012818 (set_id, dim0, dim1, param_value) VALUES (2, 100, 40, 40);
INSERT INTO salaryAge_w_2012818 (set_id, dim0, dim1, param_value) VALUES (2, 200, 10, 11);
INSERT INTO salaryAge_w_2012818 (set_id, dim0, dim1, param_value) VALUES (2, 200, 20, 21);
INSERT INTO salaryAge_w_2012818 (set_id, dim0, dim1, param_value) VALUES (2, 200, 30, 31);
INSERT INTO salaryAge_w_2012818 (set_id, dim0, dim1, param_value) VALUES (2, 200, 40, 41);
INSERT INTO salaryAge_w_2012818 (set_id, dim0, dim1, param_value) VALUES (2, 300, 10, 12);
INSERT INTO salaryAge_w_2012818 (set_id, dim0, dim1, param_value) VALUES (2, 300, 20, 22);
INSERT INTO salaryAge_w_2012818 (set_id, dim0, dim1, param_value) VALUES (2, 300, 30, 32);
INSERT INTO salaryAge_w_2012818 (set_id, dim0, dim1, param_value) VALUES (2, 300, 40, 42);

-- random values starting seed, set id = 2 
INSERT INTO StartingSeed_w_2012819 (set_id, param_value) VALUES (2, 8191);

-- full or part time by salary default values, set id = 2
-- enum ids for salary not a zero based
-- parameter type is enum based, enum ids: 22, 33
INSERT INTO salaryFull_w_2012812 (set_id, dim0, param_value) VALUES (2, 100, 33);
INSERT INTO salaryFull_w_2012812 (set_id, dim0, param_value) VALUES (2, 200, 22);
INSERT INTO salaryFull_w_2012812 (set_id, dim0, param_value) VALUES (2, 300, 22);

-- age by sex, set id = 3, enum ids for age not a zero based
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (3, 10, 0, 1.1);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (3, 10, 1, 1.2);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (3, 20, 0, 1.3);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (3, 20, 1, 1.4);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (3, 30, 0, 1.5);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (3, 30, 1, 1.6);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (3, 40, 0, 1.7);
INSERT INTO ageSex_w_2012817 (set_id, dim0, dim1, param_value) VALUES (3, 40, 1, 1.8);

-- age by sex values, set id = 4
INSERT INTO ageSex_w_2012817 
  (set_id, dim0, dim1, param_value) 
SELECT 
  4, dim0, dim1, 4 * param_value 
FROM ageSex_w_2012817 WHERE set_id = 2;

-- salary by age values, set id = 4
INSERT INTO salaryAge_w_2012818 
  (set_id, dim0, dim1, param_value) 
SELECT 
  4, dim0, dim1, param_value + 4
FROM salaryAge_w_2012818 WHERE set_id = 2;
  
-- random values starting seed, set id = 4
INSERT INTO StartingSeed_w_2012819 (set_id, param_value) VALUES (4, 4095);

-- full or part time by salary level values, set id = 4
INSERT INTO salaryFull_w_2012812
  (set_id, dim0, param_value) 
SELECT 
  4, dim0, param_value
FROM salaryFull_w_2012812 WHERE set_id = 2;
