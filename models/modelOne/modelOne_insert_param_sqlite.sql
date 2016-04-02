--
-- Copyright (c) 2013 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--
-- keep dummy sql below to prevent sqlite3 failure due to UTF-8 BOM
-- it is typical problem if .sql saved by Windows text editors
--
SELECT * FROM sqlite_master WHERE 0 = 1;

--
-- list of ids: values < 10 reserved for development and testing
-- test: model id = 1 is modelOne
-- test: set id = 2 is default set of parameters for modelOne
-- test: set id = 3 is modified working set for modelOne
--
-- INSERT INTO id_lst (id_key, id_value) VALUES ('model_id', 10);
-- INSERT INTO id_lst (id_key, id_value) VALUES ('run_id_set_id', 10);

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
INSERT INTO workset_lst (set_id, base_run_id, model_id, set_name, is_readonly, update_dt) VALUES (2, NULL, 1, 'modelOne', 1, '2013-05-29 23:55:07.1234');

INSERT INTO workset_txt(set_id, model_id, lang_id, descr, note) VALUES (2, 1, 0, 'Model One default set of parameters', NULL);
INSERT INTO workset_txt(set_id, model_id, lang_id, descr, note) VALUES (2, 1, 1, '(FR) Model One default set of parameters', NULL);

INSERT INTO workset_parameter (set_id, model_id, parameter_id) VALUES (2, 1, 0);
INSERT INTO workset_parameter (set_id, model_id, parameter_id) VALUES (2, 1, 1);
INSERT INTO workset_parameter (set_id, model_id, parameter_id) VALUES (2, 1, 2);

INSERT INTO workset_parameter_txt (set_id, model_id, parameter_id, lang_id, note) VALUES (2, 1, 0, 0, 'Age by Sex default values');
INSERT INTO workset_parameter_txt (set_id, model_id, parameter_id, lang_id, note) VALUES (2, 1, 0, 1, '(FR) Age by Sex default values');
INSERT INTO workset_parameter_txt (set_id, model_id, parameter_id, lang_id, note) VALUES (2, 1, 1, 0, 'Salary by Age default values');
INSERT INTO workset_parameter_txt (set_id, model_id, parameter_id, lang_id, note) VALUES (2, 1, 1, 1, '(FR) Salary by Age default values');
INSERT INTO workset_parameter_txt (set_id, model_id, parameter_id, lang_id, note) VALUES (2, 1, 2, 0, 'Starting seed default value');

--
-- modelOne modified set of parameters
-- use this workset for test only: it is INVALID set 
-- workset must either contain all parameters or have base run_id not null
--
INSERT INTO workset_lst (set_id, base_run_id, model_id, set_name, is_readonly, update_dt) VALUES (3, NULL, 1, 'modelOne_modified', 0, '2013-05-30 23:55:07.1234');

INSERT INTO workset_txt(set_id, model_id, lang_id, descr, note) VALUES (3, 1, 0, 'modelOne modified set of parameters', NULL);

INSERT INTO workset_parameter (set_id, model_id, parameter_id) VALUES (3, 1, 0);

INSERT INTO workset_parameter_txt (set_id, model_id, parameter_id, lang_id, note) VALUES (3, 1, 0, 0, 'Age by Sex modified values');

--
-- modelOne other set of parameters
-- it is not based on model run and therefore must include ALL model parameters
--
INSERT INTO workset_lst (set_id, base_run_id, model_id, set_name, is_readonly, update_dt) VALUES (4, NULL, 1, 'modelOne_other', 1, '2013-05-29 23:55:07.1234');

INSERT INTO workset_txt(set_id, model_id, lang_id, descr, note) VALUES (4, 1, 0, 'Model One other set of parameters', NULL);
INSERT INTO workset_txt(set_id, model_id, lang_id, descr, note) VALUES (4, 1, 1, '(FR) Model One other set of parameters', NULL);

INSERT INTO workset_parameter (set_id, model_id, parameter_id) VALUES (4, 1, 0);
INSERT INTO workset_parameter (set_id, model_id, parameter_id) VALUES (4, 1, 1);
INSERT INTO workset_parameter (set_id, model_id, parameter_id) VALUES (4, 1, 2);

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
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (2, 10, 0, 0.1);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (2, 10, 1, 0.2);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (2, 20, 0, 0.3);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (2, 20, 1, 0.4);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (2, 30, 0, 0.5);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (2, 30, 1, 0.6);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (2, 40, 0, 0.7);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (2, 40, 1, 0.8);

-- salary by age default values, set id = 2
INSERT INTO modelOne_201208171604590148_w1_salaryAge (set_id, dim0, dim1, value) VALUES (2, 0, 10, 10);
INSERT INTO modelOne_201208171604590148_w1_salaryAge (set_id, dim0, dim1, value) VALUES (2, 0, 20, 20);
INSERT INTO modelOne_201208171604590148_w1_salaryAge (set_id, dim0, dim1, value) VALUES (2, 0, 30, 30);
INSERT INTO modelOne_201208171604590148_w1_salaryAge (set_id, dim0, dim1, value) VALUES (2, 0, 40, 40);
INSERT INTO modelOne_201208171604590148_w1_salaryAge (set_id, dim0, dim1, value) VALUES (2, 1, 10, 11);
INSERT INTO modelOne_201208171604590148_w1_salaryAge (set_id, dim0, dim1, value) VALUES (2, 1, 20, 21);
INSERT INTO modelOne_201208171604590148_w1_salaryAge (set_id, dim0, dim1, value) VALUES (2, 1, 30, 31);
INSERT INTO modelOne_201208171604590148_w1_salaryAge (set_id, dim0, dim1, value) VALUES (2, 1, 40, 41);
INSERT INTO modelOne_201208171604590148_w1_salaryAge (set_id, dim0, dim1, value) VALUES (2, 2, 10, 12);
INSERT INTO modelOne_201208171604590148_w1_salaryAge (set_id, dim0, dim1, value) VALUES (2, 2, 20, 22);
INSERT INTO modelOne_201208171604590148_w1_salaryAge (set_id, dim0, dim1, value) VALUES (2, 2, 30, 32);
INSERT INTO modelOne_201208171604590148_w1_salaryAge (set_id, dim0, dim1, value) VALUES (2, 2, 40, 42);

-- random values starting seed, set id = 2 
INSERT INTO modelOne_201208171604590148_w2_StartingSeed (set_id, value) VALUES (2, 8191);

-- age by sex, set id = 3
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (3, 10, 0, 1.1);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (3, 10, 1, 1.2);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (3, 20, 0, 1.3);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (3, 20, 1, 1.4);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (3, 30, 0, 1.5);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (3, 30, 1, 1.6);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (3, 40, 0, 1.7);
INSERT INTO modelOne_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (3, 40, 1, 1.8);

-- age by sex values, set id = 4
INSERT INTO modelOne_201208171604590148_w0_ageSex 
  (set_id, dim0, dim1, value) 
SELECT 
  4, dim0, dim1, 4 * value 
FROM modelOne_201208171604590148_w0_ageSex WHERE set_id = 2;

-- salary by age values, set id = 4
INSERT INTO modelOne_201208171604590148_w1_salaryAge 
  (set_id, dim0, dim1, value) 
SELECT 
  4, dim0, dim1, value + 4
FROM modelOne_201208171604590148_w1_salaryAge WHERE set_id = 2;
  
-- random values starting seed, set id = 4
INSERT INTO modelOne_201208171604590148_w2_StartingSeed (set_id, value) VALUES (4, 4095);

