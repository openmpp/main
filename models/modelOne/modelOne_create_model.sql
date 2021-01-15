--
-- Copyright (c) 2013-2015 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--

--
-- list of ids: values < 100 reserved for development and testing
--   model id = 1 is modelOne
--   set id = 2 is default set of input parameters for modelOne
--   set id = 3 invalid set of input parameters, for test only
--   set id = 4 modified set of input parameters
--

--
-- modelOne metadata
-- model digest: not a real digest (32 digits hex)
--
INSERT INTO model_dic
  (model_id, model_name, model_digest, model_type, model_ver, create_dt, default_lang_id)
VALUES
  (1, 'modelOne', '_201208171604590148_', 0, '1.0', '2012-08-17 16:04:59.148', 0);
  
INSERT INTO model_dic_txt (model_id, lang_id, descr, note) VALUES (1, 0, 'First model', 'First model: openM++ development test model');
INSERT INTO model_dic_txt (model_id, lang_id, descr, note) VALUES (1, 1, '(FR) First model', NULL);

INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 0, 'Running Simulation', 'Running Simulation');
INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 0, 'Event loop completed', 'Event loop completed');
INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 0, 'Reading Parameters', 'Reading Parameters');
INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 0, 'Start model subvalue', 'Start model subvalue');
INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 0, 'Writing Output Tables', 'Writing Output Tables');

INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 1, 'Running Simulation', '(FR) Running Simulation');
INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 1, 'Event loop completed', '(FR) Event loop completed');

-- 
-- modelOne simple types: reference to built-in types
--
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 4, 4);
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 7, 7);
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 14, 14);
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 21, 21);

-- 
-- modelOne classifcation types
-- type digest: not a real digest (32 digits hex)
--
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (96, 'age', '_20128171604590121', 2, 500);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (97, 'sex', '_20128171604590122', 2, 800);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (98, 'salary', '_20128171604590123', 2, 400);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (99, 'full', '_20128171604590124', 2, 44);

INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 101, 96);
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 102, 97);
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 103, 98);
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 104, 99);

INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (96, 0, 'Age', NULL);
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (96, 1, '(FR) Age', NULL);
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (97, 0, 'Sex', NULL);
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (98, 0, 'Salary', NULL);
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (99, 0, 'Full or part time', NULL);

--
-- enums for model-specific types, age, salary and full-or-part enum id's not zero-based
--
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (96, 10, '10-20');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (96, 20, '20-30');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (96, 30, '30-40');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (96, 40, '40+');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (97, 0, 'M');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (97, 1, 'F');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (98, 100, 'L');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (98, 200, 'M');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (98, 300, 'H');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (99, 22, 'Full');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (99, 33, 'Part');

INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (96, 10, 0, 'age 10-20', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (96, 10, 1, '(FR) age 10-20', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (96, 20, 0, 'age 20-30', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (96, 20, 1, '(FR) age 20-30', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (96, 30, 0, 'age 30-40', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (96, 30, 1, '(FR) age 30-40', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (96, 40, 0, 'age 40+', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (96, 40, 1, '(FR) age 40+', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (97, 0, 0, 'Male', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (97, 1, 0, 'Female', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (98, 100, 0, 'Low', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (98, 200, 0, 'Medium', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (98, 300, 0, 'High', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (99, 22, 0, 'Full-time', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (99, 33, 0, 'Part-time', NULL);

-- 
-- modelOne input parameters
-- parameter digest: not a real digest (32 digits hex)
-- db suffix: not a real value (8 digits hex)
--   ageSex rank 2, float and parameter value can be null
--   salaryAge rank 2, int
--   StartingSeed rank 0 scalar (no dimensions), int
--   salaryFull rank 1, enum-based
--   baseSalary rank 0 scalar enum-based
--   filePath rank 0 of string type
--   isOldAge rank 0 logical type
--
INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, parameter_rank, type_hid, is_extendable, num_cumulated, db_run_table, db_set_table, import_digest)
VALUES
  (44, 'ageSex', '_20128171604590131', 2, 14, 1, 0, 'ageSex_p_2012817', 'ageSex_w_2012817', '_i0128171604590131');

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, parameter_rank, type_hid, is_extendable, num_cumulated, db_run_table, db_set_table, import_digest)
VALUES
  (45, 'salaryAge', '_20128171604590132', 2, 4, 0, 0, 'salaryAge_p_2012818', 'salaryAge_w_2012818', '_i0128171604590132');

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, parameter_rank, type_hid, is_extendable, num_cumulated, db_run_table, db_set_table, import_digest)
VALUES
  (46, 'StartingSeed', '_20128171604590133', 0, 4, 0, 0, 'StartingSeed_p_2012819', 'StartingSeed_w_2012819', '_i0128171604590133');

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, parameter_rank, type_hid, is_extendable, num_cumulated, db_run_table, db_set_table, import_digest)
VALUES
  (47, 'salaryFull', '_20128171604590134', 1, 99, 0, 0, 'salaryFull_p_2012812', 'salaryFull_w_2012812', '_i0128171604590134');

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, parameter_rank, type_hid, is_extendable, num_cumulated, db_run_table, db_set_table, import_digest)
VALUES
  (48, 'baseSalary', '_20128171604590135', 0, 99, 0, 0, 'baseSalary_p_2012811', 'baseSalary_w_2012811', '_i0128171604590135');

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, parameter_rank, type_hid, is_extendable, num_cumulated, db_run_table, db_set_table, import_digest)
VALUES
  (49, 'filePath', '_20128171604590136', 0, 21, 0, 0, 'filePath_p_2012814', 'filePath_w_2012814', '_i0128171604590136');

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, parameter_rank, type_hid, is_extendable, num_cumulated, db_run_table, db_set_table, import_digest)
VALUES
  (50, 'isOldAge', '_20128171604590137', 1, 7, 0, 0, 'isOldAge_p_2012815', 'isOldAge_w_2012815', '_i0128171604590137');
  
INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 0, 44, 0);
INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 1, 45, 0);
INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 2, 46, 0);
INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 3, 47, 0);
INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 4, 48, 0);
INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 5, 49, 0);
INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 6, 50, 0);

INSERT INTO model_parameter_import (model_id, model_parameter_id, from_name, from_model_name, is_sample_dim) VALUES (1, 1, 'salaryAge', 'modelOne', 0);
INSERT INTO model_parameter_import (model_id, model_parameter_id, from_name, from_model_name, is_sample_dim) VALUES (1, 2, 'StartingSeed', 'modelOne', 0);
INSERT INTO model_parameter_import (model_id, model_parameter_id, from_name, from_model_name, is_sample_dim) VALUES (1, 0, 'ageSexIncome', 'modelOne', 0);
-- INSERT INTO model_parameter_import (model_id, model_parameter_id, from_name, from_model_name, is_sample_dim) VALUES (1, 0, 'ageSex', 'modelOne', 0);

INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (44, 0, 'Age by Sex', 'Age by Sex note');
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (44, 1, '(FR) Age by Sex', NULL);
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (45, 0, 'Salary by Age', 'Salary by Age note');
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (45, 1, '(FR) Salary by Age', '(FR) Salary by Age note');
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (46, 1, 'Starting Seed', 'Random numbers generator starting seed value');
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (47, 0, 'Full or part time by Salary level', NULL);
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (48, 0, 'Base salary level', NULL);
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (49, 0, 'File path string', NULL);
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (50, 0, 'Is Old Age', 'Is Old Age notes');
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (50, 1, '(FR) Is Old Age', '(FR) Is Old Age notes');

INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (44, 0, 'dim0', 96);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (44, 1, 'dim1', 97);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (45, 0, 'dim0', 98);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (45, 1, 'dim1', 96);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (47, 0, 'dim0', 98);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (50, 0, 'dim0', 96);

INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (44, 0, 0, 'Age Dim', 'Age Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (44, 0, 1, '(FR) Age Dim', '(FR) Age Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (44, 1, 0, 'Sex Dim', 'Sex Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (44, 1, 1, 'Sex Dim', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (47, 0, 0, 'Full Dim', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (50, 0, 0, 'Age Dim', 'Age Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (50, 0, 1, '(FR) Age Dim', '(FR) Age Dim notes');

-- 
-- modelOne output tables
-- output table digest: not a real digest (32 digits hex)
-- db suffix: not a real value (8 digits hex)
--

--
-- salarySex output table
-- salary by sex output table
--
INSERT INTO table_dic 
  (table_hid, table_name, table_digest, table_rank, is_sparse, db_expr_table, db_acc_table, db_acc_all_view, import_digest) 
VALUES 
  (82, 'salarySex', '_20128171604590182', 2, 1, 'salarySex_v_2012882', 'salarySex_a_2012882', 'salarySex_d_2012882', '_i0128171604590182');

INSERT INTO model_table_dic (model_id, model_table_id, table_hid, is_user, expr_dim_pos, is_hidden) VALUES (1, 0, 82, 0, 1, 0);
  
INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (82, 0, 'Salary by Sex', 'Salary by Sex notes', 'Measure', 'Measure notes');
INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (82, 1, '(FR) Salary by Sex', '(FR) Salary by Sex notes', '(FR) Measure', NULL);

-- dim1 "sex" has total enabled, type hid = 97, enum id = 800
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (82, 0, 'dim0', 98, 0, 3);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (82, 1, 'dim1', 97, 1, 3);

INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (82, 0, 0, 'Salary Dim', 'Salary Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (82, 0, 1, '(FR) Salary Dim', '(FR) Salary Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (82, 1, 0, 'Sex Dim', 'Sex Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (82, 1, 1, '(FR) Sex Dim', NULL);

INSERT INTO table_acc 
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) 
VALUES 
  (
  82, 0, 'acc0', 0, 'value_sum()', 'A.acc_value'
  );
INSERT INTO table_acc 
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) 
VALUES 
  (
  82, 1, 'acc1', 0, 'value_count()', 
  'SELECT A1.acc_value FROM salarySex_a_2012882 A1 WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1 AND A1.acc_id = 1'
  );
INSERT INTO table_acc 
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) 
VALUES 
  (
  82, 2, 'acc2', 1, 'acc0 + acc1', 
  '(A.acc_value) + (SELECT A1.acc_value FROM salarySex_a_2012882 A1 WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1 AND A1.acc_id = 1)'
  );
  
INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (82, 0, 0, 'Sum of salary by sex', NULL);
INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (82, 1, 0, 'Count of salary by sex', NULL);
INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (82, 2, 0, 'Derived accumulator', NULL);

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (82, 0, 'expr0', -1, 'OM_AVG(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, AVG(M1.acc_value) AS expr0 FROM salarySex_a_2012882 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (82, 1, 'expr1', 4, 'OM_SUM(acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, SUM(M1.acc_value) AS expr1 FROM salarySex_a_2012882 M1 WHERE M1.acc_id = 1 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (82, 2, 'expr2', 0, 'OM_MIN(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, MIN(M1.acc_value) AS expr2 FROM salarySex_a_2012882 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (82, 3, 'expr3', 2, 'OM_AVG(acc0 * acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, AVG(M1.acc_value * A1.acc1) AS expr3 FROM salarySex_a_2012882 M1 INNER JOIN (SELECT run_id, dim0, dim1, sub_id, acc_value AS acc1 FROM salarySex_a_2012882 WHERE acc_id = 1) A1 ON (A1.run_id = M1.run_id AND A1.dim0 = M1.dim0 AND A1.dim1 = M1.dim1 AND A1.sub_id = M1.sub_id) WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
  
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (82, 0, 0, 'Average acc0', 'Average on acc0 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (82, 0, 1, '(FR) Average acc0', '(FR) Average on acc0 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (82, 1, 0, 'Sum acc1', NULL);
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (82, 2, 0, 'Min acc0', NULL);
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (82, 3, 0, 'Average acc0 * acc1', NULL);

--
-- fullAgeSalary output table
-- full time by age by salary output table
--
INSERT INTO table_dic 
  (table_hid, table_name, table_digest, table_rank, is_sparse, db_expr_table, db_acc_table, db_acc_all_view, import_digest) 
VALUES 
  (83, 'fullAgeSalary', '_20128171604590183', 3, 0, 'fullAgeSalary_v_2012883', 'fullAgeSalary_a_2012883', 'fullAgeSalary_d_2012883', '_i0128171604590183');

INSERT INTO model_table_dic (model_id, model_table_id, table_hid, is_user, expr_dim_pos, is_hidden) VALUES (1, 1, 83, 0, 1, 0);

INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (83, 0, 'Full Time by Age by Salary Bracket', 'Full Time by Age by Salary Bracket notes', 'Measure', 'Measure notes');
INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (83, 1, '(FR) Full Time by Age by Salary Bracket', '(FR) Full Time by Age by Salary Bracket notes', '(FR) Measure', NULL);

-- dim1 "age" has total enabled, type hid = 96, enum id = 500
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (83, 0, 'dim0', 99, 0, 2);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (83, 1, 'dim1', 96, 1, 5);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (83, 2, 'dim2', 98, 0, 3);

INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (83, 0, 0, 'Full Time', 'Full or Part Time Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (83, 0, 1, '(FR) Full Time', '(FR) Full or Part Time Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (83, 1, 0, 'Age Dim', 'Age Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (83, 1, 1, '(FR) Age Dim', NULL);
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (83, 2, 0, 'Salary Dim', 'Salary Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (83, 2, 1, '(FR) Salary Dim', '(FR) Salary Dim notes');

INSERT INTO table_acc (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) VALUES (83, 0, 'acc0', 0, 'raw_value()', 'A.acc_value');

INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (83, 0, 0, 'Full time salary by age', 'Full time salary by age notes');

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (83, 0, 'expr0', -1, 'OM_AVG(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, AVG(M1.acc_value) AS expr0 FROM fullAgeSalary_a_2012883 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2'
  );

INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (83, 0, 0, 'Average acc0', 'Average on acc0 notes');

--
-- ageSexIncome output table
-- age by sex income output table
-- import digest same as ageSex parameter import digest
--
INSERT INTO table_dic 
  (table_hid, table_name, table_digest, table_rank, is_sparse, db_expr_table, db_acc_table, db_acc_all_view, import_digest) 
VALUES 
  (84, 'ageSexIncome', '_20128171604590184', 2, 0, 'ageSexIncome_v_2012884', 'ageSexIncome_a_2012884', 'ageSexIncome_d_2012884', '_i0128171604590131');

INSERT INTO model_table_dic (model_id, model_table_id, table_hid, is_user, expr_dim_pos, is_hidden) VALUES (1, 2, 84, 0, 0, 0);

INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (84, 0, 'Age by Sex Income', 'Age by Sex Income notes', 'Income Measure', 'Income Measure notes');
INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (84, 1, '(FR) Age by Sex Income', '(FR) Age by Sex Income notes', '(FR) Income Measure notes', NULL);

-- total enum disabled for all dimensions to be imported into ageSex parameter
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (84, 0, 'dim0', 96, 0, 4);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (84, 1, 'dim1', 97, 0, 2);

INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (84, 0, 0, 'Age Dim', 'Age Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (84, 0, 1, '(FR) Age Dim', '(FR) Age Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (84, 1, 0, 'Sex Dim', 'Sex Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (84, 1, 1, '(FR) Sex Dim', NULL);

INSERT INTO table_acc (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) VALUES (84, 0, 'acc0', 0, 'raw_value()', 'A.acc_value');
INSERT INTO table_acc (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) VALUES (84, 1, 'acc1', 0, 'adjust_value()', 'A.acc_value');

INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (84, 0, 0, 'Income', 'Income notes');
INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (84, 1, 0, 'Income adjusted', 'Income adjusted notes');

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (84, 0, 'expr0', 2, 'OM_AVG(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, AVG(M1.acc_value) AS expr0 FROM ageSexIncome_a_2012884 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (84, 1, 'expr1', -1, 'OM_AVG(acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, AVG(M1.acc_value) AS expr1 FROM ageSexIncome_a_2012884 M1 WHERE M1.acc_id = 1 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );

INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (84, 0, 0, 'Average acc0', 'Average on acc0 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (84, 1, 0, 'Average acc1', 'Average on acc1 notes');

--
-- seedOldAge output table
--
INSERT INTO table_dic 
  (table_hid, table_name, table_digest, table_rank, is_sparse, db_expr_table, db_acc_table, db_acc_all_view, import_digest) 
VALUES 
  (85, 'seedOldAge', '_20128171604590185', 0, 0, 'seedOldAge_v_2012885', 'seedOldAge_a_2012885', 'seedOldAge_d_2012885', '_i0128171604590185');

INSERT INTO model_table_dic (model_id, model_table_id, table_hid, is_user, expr_dim_pos, is_hidden) VALUES (1, 3, 85, 0, 0, 0);

INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (85, 0, 'Seed Old Age', 'Seed Old Age notes', 'Seed Old Age Measure', 'Measure notes');
INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (85, 1, '(FR) Seed Old Age', '(FR) Seed Old Age notes', '(FR) Measure notes', NULL);

-- scalar output table: no dimensions

INSERT INTO table_acc (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) VALUES (85, 0, 'acc0', 0, 'raw_value()', 'A.acc_value');

INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (85, 0, 0, 'Seed', 'Seed notes');

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (85, 0, 'expr0', 5, 'OM_AVG(acc0)', 
  'SELECT M1.run_id, AVG(M1.acc_value) AS expr0 FROM seedOldAge_a_2012885 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id'
  );

INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (85, 0, 0, 'Average acc0', 'Average on acc0 notes');

--
-- model groups of parameters
--
INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden) VALUES (1, 1, 1, 'AllParameters', 0);
INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden) VALUES (1, 2, 1, 'AgeSexParameters', 0);
INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden) VALUES (1, 3, 1, 'SalaryParameters', 0);

INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 1, 0, 'All parameters', 'All model parameters group');
INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 1, 1, '(FR) All parameters', NULL);
INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 2, 0, 'Age and Sex parameters', 'Age and Sex model parameters group');
INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 2, 1, '(FR) Age and Sex parameters', '(FR) Age and Sex model parameters group');
INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 3, 0, 'Salary parameters', 'Salary model parameters group');

INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 1, 0, 2, NULL);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 1, 1, 3, NULL);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 1, 2, NULL, 2);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 1, 3, NULL, 5);

INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 2, 0, NULL, 0);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 2, 1, NULL, 1);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 2, 2, NULL, 6);

INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 3, 0, NULL, 1);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 3, 1, NULL, 3);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 3, 2, NULL, 4);

--
-- model groups of output tables
--
INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden) VALUES (1, 10, 0, 'AdditionalTables', 0);

INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 10, 0, 'Additional output tables', 'Additional output tables group notes');
INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 10, 1, '(FR) Additional output tables', NULL);

INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 10, 0, NULL, 1);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 10, 1, NULL, 2);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 10, 2, NULL, 3);

--
-- modelOne input parameters
-- enum ids for ageSex.dim0:     10,  20,  30, 40
-- enum ids for ageSex.dim1:     0,   1
-- enum ids for salaryAge.dim0:  100, 200, 300
-- enum ids for salaryAge.dim1:  10,  20,  30, 40
-- enum ids for isOldAge.dim0:   10,  20,  30, 40
-- enum ids for salaryFull.dim0: 100, 200, 300
-- enum ids for salaryFull.param_value: 22, 33
-- enum ids for baseSalary.param_value: 22, 33
--
CREATE TABLE ageSex_p_2012817 
(
  run_id      INT   NOT NULL,
  sub_id      INT   NOT NULL, 
  dim0        INT   NOT NULL, 
  dim1        INT   NOT NULL, 
  param_value FLOAT NULL,     -- float parameter value can be null
  PRIMARY KEY (run_id, sub_id, dim0, dim1)
);

CREATE TABLE ageSex_w_2012817
(
  set_id      INT   NOT NULL,
  sub_id      INT   NOT NULL, 
  dim0        INT   NOT NULL, 
  dim1        INT   NOT NULL, 
  param_value FLOAT NULL,     -- float parameter value can be null
  PRIMARY KEY (set_id, sub_id, dim0, dim1)
);

CREATE TABLE salaryAge_p_2012818
(
  run_id      INT NOT NULL,
  sub_id      INT NOT NULL, 
  dim0        INT NOT NULL, 
  dim1        INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (run_id, sub_id, dim0, dim1)
);

CREATE TABLE salaryAge_w_2012818
(
  set_id      INT NOT NULL,
  sub_id      INT NOT NULL, 
  dim0        INT NOT NULL, 
  dim1        INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (set_id, sub_id, dim0, dim1)
);

CREATE TABLE StartingSeed_p_2012819
(
  run_id      INT NOT NULL,
  sub_id      INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (run_id, sub_id)
);

CREATE TABLE StartingSeed_w_2012819
(
  set_id      INT NOT NULL,
  sub_id      INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (set_id, sub_id)
);

CREATE TABLE salaryFull_p_2012812
(
  run_id      INT NOT NULL,
  sub_id      INT NOT NULL, 
  dim0        INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (run_id, sub_id, dim0)
);

CREATE TABLE salaryFull_w_2012812
(
  set_id      INT NOT NULL,
  sub_id      INT NOT NULL, 
  dim0        INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (set_id, sub_id, dim0)
);

CREATE TABLE baseSalary_p_2012811
(
  run_id      INT NOT NULL,
  sub_id      INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (run_id, sub_id)
);

CREATE TABLE baseSalary_w_2012811
(
  set_id      INT NOT NULL,
  sub_id      INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (set_id, sub_id)
);

CREATE TABLE filePath_p_2012814
(
  run_id      INT          NOT NULL,
  sub_id      INT          NOT NULL, 
  param_value VARCHAR(255) NOT NULL,
  PRIMARY KEY (run_id, sub_id)
);

CREATE TABLE filePath_w_2012814
(
  set_id      INT          NOT NULL,
  sub_id      INT          NOT NULL, 
  param_value VARCHAR(255) NOT NULL,
  PRIMARY KEY (set_id, sub_id)
);

CREATE TABLE isOldAge_p_2012815 
(
  run_id      INT      NOT NULL,
  sub_id      INT      NOT NULL, 
  dim0        INT      NOT NULL, 
  param_value SMALLINT NOT NULL,
  PRIMARY KEY (run_id, sub_id, dim0)
);

CREATE TABLE isOldAge_w_2012815
(
  set_id      INT      NOT NULL,
  sub_id      INT      NOT NULL, 
  dim0        INT      NOT NULL, 
  param_value SMALLINT NOT NULL,
  PRIMARY KEY (set_id, sub_id, dim0)
);

--
-- modelOne output tables
--

--
-- salarySex output table
--
-- enum ids for salarySex.dim0: 100, 200, 300
-- enum ids for salarySex.dim1: 0,   1,   800
--
CREATE TABLE salarySex_a_2012882
(
  run_id    INT   NOT NULL,
  acc_id    INT   NOT NULL, 
  sub_id    INT   NOT NULL, 
  dim0      INT   NOT NULL, 
  dim1      INT   NOT NULL, 
  acc_value FLOAT NULL,
  PRIMARY KEY (run_id, acc_id, sub_id, dim0, dim1)
);

CREATE TABLE salarySex_v_2012882
(
  run_id     INT   NOT NULL,
  expr_id    INT   NOT NULL, 
  dim0       INT   NOT NULL, 
  dim1       INT   NOT NULL, 
  expr_value FLOAT NULL,
  PRIMARY KEY (run_id, expr_id, dim0, dim1)
);

--
-- fullAgeSalary output table
--
-- enum ids for fullAgeSalary.dim0: 22,  33
-- enum ids for fullAgeSalary.dim1: 10,  20,  30, 40, 500
-- enum ids for fullAgeSalary.dim2: 100, 200, 300
--
CREATE TABLE fullAgeSalary_a_2012883
(
  run_id    INT   NOT NULL,
  acc_id    INT   NOT NULL, 
  sub_id    INT   NOT NULL, 
  dim0      INT   NOT NULL, 
  dim1      INT   NOT NULL, 
  dim2      INT   NOT NULL, 
  acc_value FLOAT NULL,
  PRIMARY KEY (run_id, acc_id, sub_id, dim0, dim1, dim2)
);

CREATE TABLE fullAgeSalary_v_2012883
(
  run_id     INT   NOT NULL,
  expr_id    INT   NOT NULL, 
  dim0       INT   NOT NULL, 
  dim1       INT   NOT NULL, 
  dim2       INT   NOT NULL, 
  expr_value FLOAT NULL,
  PRIMARY KEY (run_id, expr_id, dim0, dim1, dim2)
);

--
-- ageSexIncome output table
--
-- enum ids for ageSexIncome.dim0: 10,  20,  30, 40
-- enum ids for ageSexIncome.dim1: 0,   1
--
CREATE TABLE ageSexIncome_a_2012884
(
  run_id    INT   NOT NULL,
  acc_id    INT   NOT NULL, 
  sub_id    INT   NOT NULL, 
  dim0      INT   NOT NULL, 
  dim1      INT   NOT NULL, 
  acc_value FLOAT NULL,
  PRIMARY KEY (run_id, acc_id, sub_id, dim0, dim1)
);

CREATE TABLE ageSexIncome_v_2012884
(
  run_id     INT   NOT NULL,
  expr_id    INT   NOT NULL, 
  dim0       INT   NOT NULL, 
  dim1       INT   NOT NULL, 
  expr_value FLOAT NULL,
  PRIMARY KEY (run_id, expr_id, dim0, dim1)
);

--
-- seedOldAge output table
--
-- scalar output table: no dimensions
--
CREATE TABLE seedOldAge_a_2012885
(
  run_id    INT   NOT NULL,
  acc_id    INT   NOT NULL, 
  sub_id    INT   NOT NULL, 
  acc_value FLOAT NULL,
  PRIMARY KEY (run_id, acc_id, sub_id)
);

CREATE TABLE seedOldAge_v_2012885
(
  run_id     INT   NOT NULL,
  expr_id    INT   NOT NULL, 
  expr_value FLOAT NULL,
  PRIMARY KEY (run_id, expr_id)
);

--
-- modelOne all accumulators view
--

-- if MSSQL return an error on CREATE VIEW then uncomment next line GO
--
-- GO

--
-- salarySex all accumulators view
--
-- it does include all "native" accumulators: acc0, acc1
-- and "derived" accumulator: acc2 = acc0 + acc1
--
CREATE VIEW salarySex_d_2012882
AS
SELECT
  A.run_id,
  A.sub_id,
  A.dim0,
  A.dim1,
  A.acc_value AS acc0,
  (
    SELECT A1.acc_value FROM salarySex_a_2012882 A1
    WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1
    AND A1.acc_id = 1
  ) AS acc1,
  (
    (
      A.acc_value
    )
    + 
    (
      SELECT A1.acc_value FROM salarySex_a_2012882 A1
      WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1
      AND A1.acc_id = 1
    )
  ) AS acc2
FROM salarySex_a_2012882 A
WHERE A.acc_id = 0;

--
-- salarySex all accumulators view
--
-- only "native" accumulator defined for that table: acc0
--
CREATE VIEW fullAgeSalary_d_2012883
AS
SELECT
  A.run_id,
  A.sub_id,
  A.dim0,
  A.dim1,
  A.dim2,
  A.acc_value AS acc0
FROM fullAgeSalary_a_2012883 A
WHERE A.acc_id = 0;

--
-- ageSexIncome all accumulators view
--
-- it does include all "native" accumulators: acc0, acc1
-- and "derived" accumulator: acc2 = acc0 - acc1
--
CREATE VIEW ageSexIncome_d_2012884
AS
SELECT
  A.run_id,
  A.sub_id,
  A.dim0,
  A.dim1,
  A.acc_value AS acc0,
  (
    SELECT A1.acc_value FROM ageSexIncome_a_2012884 A1
    WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1
    AND A1.acc_id = 1
  ) AS acc1,
  (
    (
      A.acc_value
    )
    - 
    (
      SELECT A1.acc_value FROM ageSexIncome_a_2012884 A1
      WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1
      AND A1.acc_id = 1
    )
  ) AS acc2
FROM ageSexIncome_a_2012884 A
WHERE A.acc_id = 0;

--
-- seedOldAge all accumulators view
--
-- only "native" accumulator defined for that table: acc0
--
CREATE VIEW seedOldAge_d_2012885
AS
SELECT
  A.run_id,
  A.sub_id,
  A.acc_value AS acc0
FROM seedOldAge_a_2012885 A
WHERE A.acc_id = 0;
