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
-- INSERT INTO profile_option (profile_name, option_key, option_value) VALUES ('modelOne', 'OpenM.Threads', '4');

--
-- modelOne default set of parameters: must include ALL model parameters
--
INSERT INTO workset_lst 
  (set_id, base_run_id, model_id, set_name, is_readonly, update_dt) 
VALUES 
  (2, NULL, 1, 'Default', 1, '2013-05-29 23:55:07.123');

INSERT INTO workset_txt(set_id, lang_id, descr, note) VALUES (2, 0, 'Model One default set of parameters', NULL);
INSERT INTO workset_txt(set_id, lang_id, descr, note) VALUES (2, 1, '(FR) Model One default set of parameters', NULL);

INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 4, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 5, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 6, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 7, 4, 3);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 8, 4, 3);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 9, 4, 3);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 10, 4, 3);

INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 4, 0, 'Age by Sex default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 4, 1, '(FR) Age by Sex default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 5, 0, 'Salary by Age default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 5, 1, '(FR) Salary by Age default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 6, 0, 'Starting seed default value');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 7, 0, 'Full or part time by Salary default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 10, 0, 'Is old age default values');

--
-- modelOne modified set of parameters
-- use this workset for test only: it is INVALID set 
-- workset must either contain all parameters or have base run_id not null
--
INSERT INTO workset_lst 
  (set_id, base_run_id, model_id, set_name, is_readonly, update_dt) 
VALUES 
  (3, NULL, 1, 'modelOne_set', 0, '2013-05-30 23:55:07.123');

INSERT INTO workset_txt(set_id, lang_id, descr, note) VALUES (3, 0, 'modelOne modified set of parameters', NULL);

INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (3, 4, 1, 0);

INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (3, 4, 0, 'Age by Sex modified values');

--
-- modelOne other set of parameters
-- it is not based on model run and therefore must include ALL model parameters
--
INSERT INTO workset_lst 
  (set_id, base_run_id, model_id, set_name, is_readonly, update_dt) 
VALUES 
  (4, NULL, 1, 'modelOne_other', 1, '2013-05-29 23:55:07.123');

INSERT INTO workset_txt(set_id, lang_id, descr, note) VALUES (4, 0, 'Model One other set of parameters', NULL);
INSERT INTO workset_txt(set_id, lang_id, descr, note) VALUES (4, 1, '(FR) Model One other set of parameters', NULL);

INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 4, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 5, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 6, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 7, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 8, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 9, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 10, 1, 0);

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

-- set id = 2
-- age by sex default values
-- enum ids for age not a zero based
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 10, 0, 0.1);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 10, 1, 0.2);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 20, 0, 0.3);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 20, 1, 0.4);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 30, 0, 0.5);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 30, 1, 0.6);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 40, 0, 0.7);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 40, 1, 0.8);

-- set id = 2
-- salary by age default values
-- enum ids for age and salary not a zero based
INSERT INTO salaryAge_w_2012818 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 100, 10, 10);
INSERT INTO salaryAge_w_2012818 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 100, 20, 20);
INSERT INTO salaryAge_w_2012818 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 100, 30, 30);
INSERT INTO salaryAge_w_2012818 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 100, 40, 40);
INSERT INTO salaryAge_w_2012818 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 200, 10, 11);
INSERT INTO salaryAge_w_2012818 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 200, 20, 21);
INSERT INTO salaryAge_w_2012818 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 200, 30, 31);
INSERT INTO salaryAge_w_2012818 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 200, 40, 41);
INSERT INTO salaryAge_w_2012818 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 300, 10, 12);
INSERT INTO salaryAge_w_2012818 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 300, 20, 22);
INSERT INTO salaryAge_w_2012818 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 300, 30, 32);
INSERT INTO salaryAge_w_2012818 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 0, 300, 40, 42);

-- set id = 2
-- random values starting seed
INSERT INTO StartingSeed_w_2012819 (set_id, sub_id, param_value) VALUES (2, 0, 8191);

-- set id = 2
-- parameter type is enum based, enum ids: 22, 33
-- number of parameter sub values: sub_count = 4
INSERT INTO baseSalary_w_2012811 (set_id, sub_id, param_value) VALUES (2, 0, 22);
INSERT INTO baseSalary_w_2012811 (set_id, sub_id, param_value) VALUES (2, 1, 33);
INSERT INTO baseSalary_w_2012811 (set_id, sub_id, param_value) VALUES (2, 2, 22);
INSERT INTO baseSalary_w_2012811 (set_id, sub_id, param_value) VALUES (2, 3, 33);

-- set id = 2
-- full or part time by salary default values
-- enum ids for salary not a zero based
-- parameter type is enum based, enum ids: 22, 33
-- number of parameter sub values: sub_count = 4
INSERT INTO salaryFull_w_2012812 (set_id, sub_id, dim0, param_value) VALUES (2, 0, 100, 33);
INSERT INTO salaryFull_w_2012812 (set_id, sub_id, dim0, param_value) VALUES (2, 0, 200, 22);
INSERT INTO salaryFull_w_2012812 (set_id, sub_id, dim0, param_value) VALUES (2, 0, 300, 22);
INSERT INTO salaryFull_w_2012812 (set_id, sub_id, dim0, param_value) VALUES (2, 1, 100, 33);
INSERT INTO salaryFull_w_2012812 (set_id, sub_id, dim0, param_value) VALUES (2, 1, 200, 33);
INSERT INTO salaryFull_w_2012812 (set_id, sub_id, dim0, param_value) VALUES (2, 1, 300, 33);
INSERT INTO salaryFull_w_2012812 (set_id, sub_id, dim0, param_value) VALUES (2, 2, 100, 22);
INSERT INTO salaryFull_w_2012812 (set_id, sub_id, dim0, param_value) VALUES (2, 2, 200, 22);
INSERT INTO salaryFull_w_2012812 (set_id, sub_id, dim0, param_value) VALUES (2, 2, 300, 22);
INSERT INTO salaryFull_w_2012812 (set_id, sub_id, dim0, param_value) VALUES (2, 3, 100, 22);
INSERT INTO salaryFull_w_2012812 (set_id, sub_id, dim0, param_value) VALUES (2, 3, 200, 22);
INSERT INTO salaryFull_w_2012812 (set_id, sub_id, dim0, param_value) VALUES (2, 3, 300, 33);

-- set id = 2
-- file path parameter type is string
-- number of parameter sub values: sub_count = 4
INSERT INTO filePath_w_2012814 (set_id, sub_id, param_value) VALUES (2, 0, 'file 0 path');
INSERT INTO filePath_w_2012814 (set_id, sub_id, param_value) VALUES (2, 1, 'file 1 path');
INSERT INTO filePath_w_2012814 (set_id, sub_id, param_value) VALUES (2, 2, 'file 2 path');
INSERT INTO filePath_w_2012814 (set_id, sub_id, param_value) VALUES (2, 3, 'file 3 path');

-- set id = 2
-- is old age default values
-- enum ids for age not a zero based
-- number of parameter sub values: sub_count = 4
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 0, 10, 0);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 0, 20, 0);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 0, 30, 0);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 0, 40, 1);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 1, 10, 0);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 1, 20, 0);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 1, 30, 0);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 1, 40, 1);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 2, 10, 0);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 2, 20, 0);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 2, 30, 0);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 2, 40, 1);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 3, 10, 0);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 3, 20, 0);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 3, 30, 0);
INSERT INTO isOldAge_w_2012815 (set_id, sub_id, dim0, param_value) VALUES (2, 3, 40, 1);

-- set id = 3
-- age by sex
-- enum ids for age not a zero based
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 0, 10, 0, 1.1);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 0, 10, 1, 1.2);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 0, 20, 0, 1.3);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 0, 20, 1, 1.4);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 0, 30, 0, 1.5);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 0, 30, 1, 1.6);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 0, 40, 0, 1.7);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 0, 40, 1, 1.8);

-- set id = 4
-- age by sex values
INSERT INTO ageSex_w_2012817 
  (set_id, sub_id, dim0, dim1, param_value) 
SELECT 
  4, sub_id, dim0, dim1, 4 * param_value 
FROM ageSex_w_2012817 WHERE set_id = 2;

-- set id = 4
-- salary by age values
INSERT INTO salaryAge_w_2012818 
  (set_id, sub_id, dim0, dim1, param_value) 
SELECT 
  4, sub_id, dim0, dim1, param_value + 4
FROM salaryAge_w_2012818 WHERE set_id = 2;
  
-- set id = 4
-- random values starting seed
INSERT INTO StartingSeed_w_2012819 (set_id, sub_id, param_value) VALUES (4, 0, 4095);

-- set id = 4
-- full or part time by salary level values
INSERT INTO salaryFull_w_2012812
  (set_id, sub_id, dim0, param_value) 
SELECT 
  4, sub_id, dim0, param_value
FROM salaryFull_w_2012812 WHERE set_id = 2 AND sub_id = 0;

-- set id = 4
-- parameter type is enum based, enum ids: 22, 33
INSERT INTO baseSalary_w_2012811 (set_id, sub_id, param_value) VALUES (4, 0, 33);

-- set id = 4
-- file path parameter type is string
INSERT INTO filePath_w_2012814 (set_id, sub_id, param_value) VALUES (4, 0, 'set 4 file 0 path');

-- set id = 4
-- is old age values
INSERT INTO isOldAge_w_2012815
  (set_id, sub_id, dim0, param_value) 
SELECT 
  4, sub_id, dim0, param_value
FROM isOldAge_w_2012815 WHERE set_id = 2 AND sub_id = 0;

