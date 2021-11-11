--
-- Copyright (c) 2013 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--

--
-- list of ids: values < 10 reserved for development and testing
--   model id = 1 is modelOne
--   set id = 2 is default set of input parameters for modelOne
--   set id = 3 partial set of input parameters, not based on run and contain only one parameter value
--   set id = 4 modified set of input parameters
--   set id = 5 read-write set of input parameters, not based on run and contain only one parameter value
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

INSERT INTO workset_txt (set_id, lang_id, descr, note) VALUES (2, 0, 'Model One default set of parameters', NULL);
INSERT INTO workset_txt (set_id, lang_id, descr, note) VALUES (2, 1, 'Modèle Un ensemble de paramètres par défaut', NULL);

INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 101, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 102, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 103, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 104, 4, 3);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 105, 4, 3);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 106, 4, 3);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (2, 107, 4, 3);

INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 101, 0, 'Age by Sex default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 101, 1, 'Valeurs par défaut de l''Âge par Sexe');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 102, 0, 'Salary by Age default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 102, 1, 'Salaire par Âge valeurs par défaut');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 103, 0, 'Starting seed default value');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 104, 0, 'Full or part time by Salary default values');
INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (2, 107, 0, 'Is old age default values');

--
-- modelOne partial set of parameters
-- it contain only one parameter value Age by Sex with default sub id 32
--
INSERT INTO workset_lst 
  (set_id, base_run_id, model_id, set_name, is_readonly, update_dt) 
VALUES 
  (3, NULL, 1, 'modelOne_partial', 1, '2013-05-30 23:55:07.123');

INSERT INTO workset_txt (set_id, lang_id, descr, note) VALUES (3, 0, 'modelOne partial set of parameters', NULL);

INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (3, 101, 1, 32);

INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (3, 101, 0, 'Age by Sex partial values');

--
-- modelOne other set of parameters
-- it is not based on model run and does include ALL model parameters
--
INSERT INTO workset_lst 
  (set_id, base_run_id, model_id, set_name, is_readonly, update_dt) 
VALUES 
  (4, NULL, 1, 'modelOne_other', 1, '2013-05-29 23:55:07.123');

INSERT INTO workset_txt (set_id, lang_id, descr, note) VALUES (4, 0, 'Model One other set of parameters', NULL);
INSERT INTO workset_txt (set_id, lang_id, descr, note) VALUES (4, 1, 'Modèle Un autre ensemble de paramètres', NULL);

INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 101, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 102, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 103, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 104, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 105, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 106, 1, 0);
INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (4, 107, 1, 0);

--
-- modelOne updated set of parameters, it contain only one parameter value
-- it is read-write workset
--
INSERT INTO workset_lst 
  (set_id, base_run_id, model_id, set_name, is_readonly, update_dt) 
VALUES 
  (5, NULL, 1, 'modelOne_updated', 0, '2013-05-30 23:55:07.123');

INSERT INTO workset_txt (set_id, lang_id, descr, note) VALUES (5, 0, 'modelOne updated set of parameters', NULL);

INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id) VALUES (5, 102, 1, 0);

INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note) VALUES (5, 102, 0, 'Salary by Age updated values');

--
-- modelOne modelling tasks
--
INSERT INTO task_lst (task_id, model_id, task_name) VALUES (1, 1, 'taskOne');

INSERT INTO task_txt (task_id, lang_id, descr, note) VALUES (1, 0, 'Task One for Model One', 'Task One: two set of input parameters');
INSERT INTO task_txt (task_id, lang_id, descr, note) VALUES (1, 1, 'Tâche Un pour le Modèle Un', NULL);

INSERT INTO task_set (task_id, set_id) VALUES (1, 2);
INSERT INTO task_set (task_id, set_id) VALUES (1, 4);

--
-- modelOne input parameters
--

--
-- set id = 2
--
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

-- random values starting seed
INSERT INTO StartingSeed_w_2012819 (set_id, sub_id, param_value) VALUES (2, 0, 8191);

-- parameter type is enum based, enum ids: 22, 33
-- number of parameter sub values: sub_count = 4
INSERT INTO baseSalary_w_2012811 (set_id, sub_id, param_value) VALUES (2, 0, 22);
INSERT INTO baseSalary_w_2012811 (set_id, sub_id, param_value) VALUES (2, 1, 33);
INSERT INTO baseSalary_w_2012811 (set_id, sub_id, param_value) VALUES (2, 2, 22);
INSERT INTO baseSalary_w_2012811 (set_id, sub_id, param_value) VALUES (2, 3, 33);

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

-- file path parameter type is string
-- number of parameter sub values: sub_count = 4
INSERT INTO filePath_w_2012814 (set_id, sub_id, param_value) VALUES (2, 0, 'file 0 path');
INSERT INTO filePath_w_2012814 (set_id, sub_id, param_value) VALUES (2, 1, 'file 1 path');
INSERT INTO filePath_w_2012814 (set_id, sub_id, param_value) VALUES (2, 2, 'file 2 path');
INSERT INTO filePath_w_2012814 (set_id, sub_id, param_value) VALUES (2, 3, 'file 3 path');

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



--
-- set id = 3
--
-- age by sex
-- enum ids for age not a zero based
-- default sub-value id = 32
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 32, 10, 0, 32.1);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 32, 10, 1, 32.2);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 32, 20, 0, 32.3);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 32, 20, 1, 32.4);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 32, 30, 0, 32.5);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 32, 30, 1, 32.6);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 32, 40, 0, 32.7);
INSERT INTO ageSex_w_2012817 (set_id, sub_id, dim0, dim1, param_value) VALUES (3, 32, 40, 1, 32.8);


--
-- set id = 4
--
-- age by sex values
INSERT INTO ageSex_w_2012817 
  (set_id, sub_id, dim0, dim1, param_value) 
SELECT 
  4, sub_id, dim0, dim1, 4 * param_value 
FROM ageSex_w_2012817 WHERE set_id = 2;

-- salary by age values
INSERT INTO salaryAge_w_2012818 
  (set_id, sub_id, dim0, dim1, param_value) 
SELECT 
  4, sub_id, dim0, dim1, param_value + 4
FROM salaryAge_w_2012818 WHERE set_id = 2;
  
-- random values starting seed
INSERT INTO StartingSeed_w_2012819 (set_id, sub_id, param_value) VALUES (4, 0, 4095);

-- full or part time by salary level values
INSERT INTO salaryFull_w_2012812
  (set_id, sub_id, dim0, param_value) 
SELECT 
  4, sub_id, dim0, param_value
FROM salaryFull_w_2012812 WHERE set_id = 2 AND sub_id = 0;

-- parameter type is enum based, enum ids: 22, 33
INSERT INTO baseSalary_w_2012811 (set_id, sub_id, param_value) VALUES (4, 0, 33);

-- set id = 4
-- file path parameter type is string
INSERT INTO filePath_w_2012814 (set_id, sub_id, param_value) VALUES (4, 0, 'set 4 file 0 path');

-- is old age values
INSERT INTO isOldAge_w_2012815
  (set_id, sub_id, dim0, param_value) 
SELECT 
  4, sub_id, dim0, param_value
FROM isOldAge_w_2012815 WHERE set_id = 2 AND sub_id = 0;


--
-- set id = 5
--
-- salary by age values
INSERT INTO salaryAge_w_2012818 
  (set_id, sub_id, dim0, dim1, param_value) 
SELECT 
  5, sub_id, dim0, dim1, param_value + 32
FROM salaryAge_w_2012818 WHERE set_id = 2;
  