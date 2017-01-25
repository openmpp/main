--
-- Copyright (c) 2013-2015 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--

--
-- list of ids: values < 10 reserved for development and testing
--   model id = 1 is modelOne
--   set id = 2 is default set of input parameters for modelOne
--   set id = 3, 4 modified working sets of input parameters
--

--
-- modelOne metadata
-- model digest: not a real digest (32 digits hex)
--
INSERT INTO model_dic
  (model_id, model_name, model_digest, model_type, model_ver, create_dt, default_lang_id)
VALUES
  (1, 'modelOne', '_201208171604590148_', 0, '1.0', '2012-08-17 16:04:59.0148', 0);
  
INSERT INTO model_dic_txt (model_id, lang_id, descr, note) VALUES (1, 0, 'First model', NULL);
INSERT INTO model_dic_txt (model_id, lang_id, descr, note) VALUES (1, 1, '(FR) First model', NULL);

INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 0, 'Running Simulation', 'Running Simulation');
INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 0, 'Event loop completed', 'Event loop completed');
INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 0, 'Reading Parameters', 'Reading Parameters');
INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 0, 'Start model subsample', 'Start model subsample');
INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 0, 'Writing Output Tables', 'Writing Output Tables');

INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 1, 'Running Simulation', '(FR) Running Simulation');
INSERT INTO model_word (model_id, lang_id, word_code, word_value) VALUES (1, 1, 'Event loop completed', '(FR) Event loop completed');

-- 
-- modelOne simple types: reference to built-in types
--
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 4, 4);
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 14, 14);

-- 
-- modelOne classifcation types
-- type digest: not a real digest (32 digits hex)
--
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (96, 'age', '_20128171604590121', 2, 50);
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
-- salaryFull parameter type is enum-based
--
INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, db_run_table, db_set_table, parameter_rank, type_hid, num_cumulated)
VALUES
  (4, 'ageSex', '_20128171604590131', 'ageSex_p_2012817', 'ageSex_w_2012817', 2, 14, 0);

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, db_run_table, db_set_table, parameter_rank, type_hid, num_cumulated)
VALUES
  (5, 'salaryAge', '_20128171604590132', 'salaryAge_p_2012818', 'salaryAge_w_2012818', 2, 4, 0);

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, db_run_table, db_set_table, parameter_rank, type_hid, num_cumulated)
VALUES
  (6, 'StartingSeed', '_20128171604590133', 'StartingSeed_p_2012819', 'StartingSeed_w_2012819', 0, 4, 0);

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, db_run_table, db_set_table, parameter_rank, type_hid, num_cumulated)
VALUES
  (7, 'salaryFull', '_20128171604590134', 'salaryFull_p_2012812', 'salaryFull_w_2012812', 1, 99, 0);

INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 0, 4, 0);
INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 1, 5, 0);
INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 2, 6, 0);
INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 3, 7, 0);

INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (4, 0, 'Age by Sex', 'Age by Sex note');
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (4, 1, '(FR) Age by Sex', NULL);
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (5, 0, 'Salary by Age', 'Salary by Age note');
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (5, 1, '(FR) Salary by Age', '(FR) Salary by Age note');
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (6, 1, 'Starting Seed', 'Random numbers generator starting seed value');
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (7, 0, 'Full or part time by Salary level', NULL);

INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (4, 0, 'dim0', 96);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (4, 1, 'dim1', 97);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (5, 0, 'dim0', 98);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (5, 1, 'dim1', 96);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (7, 0, 'dim0', 98);

INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (4, 0, 0, 'Age Dim', 'Age Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (4, 0, 1, '(FR) Age Dim', '(FR) Age Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (4, 1, 0, 'Sex Dim', 'Sex Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (4, 1, 1, 'Sex Dim', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (7, 0, 0, 'Full Dim', NULL);

-- 
-- modelOne output tables
-- output table digest: not a real digest (32 digits hex)
-- db suffix: not a real value (8 digits hex)
--
INSERT INTO table_dic 
  (table_hid, table_name, table_digest, table_rank, is_sparse, db_expr_table, db_acc_table, db_acc_all_view) 
VALUES 
  (2, 'salarySex', '_20128171604590141', 2, 1, 'salarySex_v_2012820', 'salarySex_a_2012820', 'salarySex_d_2012820');

INSERT INTO model_table_dic (model_id, model_table_id, table_hid, is_user, expr_dim_pos) VALUES (1, 0, 2, 0, 1);
  
INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (2, 0, 'Salary by Sex', 'Salary by Sex notes', 'Salary Dim', NULL);
INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (2, 1, '(FR) Salary by Sex', '(FR) Salary by Sex notes', '(FR) Salary Dim', NULL);

-- dim1 "sex" has total enabled, enum id = 800
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (2, 0, 'dim0', 98, 0, 3);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (2, 1, 'dim1', 97, 1, 3);

INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (2, 0, 0, 'Salary Dim', 'Salary Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (2, 0, 1, '(FR) Salary Dim', '(FR) Salary Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (2, 1, 0, 'Sex Dim', 'Sex Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (2, 1, 1, '(FR) Sex Dim', NULL);

INSERT INTO table_acc (table_hid, acc_id, acc_name, is_derived, acc_expr) VALUES (2, 0, 'acc0', 0, 'value_sum()');
INSERT INTO table_acc (table_hid, acc_id, acc_name, is_derived, acc_expr) VALUES (2, 1, 'acc1', 0, 'value_count()');
INSERT INTO table_acc (table_hid, acc_id, acc_name, is_derived, acc_expr) VALUES (2, 2, 'acc2', 1, 'acc0 + acc1');

INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (2, 0, 0, 'Sum of salary by sex', NULL);
INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (2, 1, 0, 'Count of salary by sex', NULL);

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (2, 0, 'expr0', 4, 'OM_AVG(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, AVG(M1.acc_value) AS expr0 FROM salarySex_a_2012820 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (2, 1, 'expr1', 4, 'OM_SUM(acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, SUM(M1.acc_value) AS expr1 FROM salarySex_a_2012820 M1 WHERE M1.acc_id = 1 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (2, 2, 'expr2', 2, 'OM_MIN(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, MIN(M1.acc_value) AS expr2 FROM salarySex_a_2012820 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (2, 3, 'expr3', 3, 'OM_AVG(acc0 * acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, AVG(M1.acc_value * A1.acc1) AS expr3 FROM salarySex_a_2012820 M1 INNER JOIN (SELECT run_id, dim0, dim1, sub_id, acc_value AS acc1 FROM salarySex_a_2012820 WHERE acc_id = 1) A1 ON (A1.run_id = M1.run_id AND A1.dim0 = M1.dim0 AND A1.dim1 = M1.dim1 AND A1.sub_id = M1.sub_id) WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
  
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (2, 0, 0, 'Average acc0', 'Average on acc0 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (2, 0, 1, '(FR) Average acc0', '(FR) Average on acc0 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (2, 1, 0, 'Sum acc1', NULL);
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (2, 2, 0, 'Min acc0', NULL);
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (2, 3, 0, 'Average acc0 * acc1', NULL);

INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden) VALUES (1, 1, 1, 'all_parameters', 0);
INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden) VALUES (1, 2, 1, 'age_sex_parameters', 0);

INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 1, 0, 'All parameters', 'All model parameters group');
INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 1, 1, '(FR) All parameters', NULL);
INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 2, 0, 'Age and Sex parameters', 'Age and Sex model parameters group');

INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 1, 1, 2, NULL);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 1, 2, NULL, 2);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 1, 3, NULL, 3);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 2, 1, NULL, 0);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 2, 2, NULL, 1);

--
-- modelOne input parameters
-- enum ids for ageSex.dim0:     10,  20,  30, 40
-- enum ids for ageSex.dim1:     0,   1
-- enum ids for salaryAge.dim0:  100, 200, 300
-- enum ids for salaryAge.dim1:  10,  20,  30, 40
-- enum ids for salaryFull.dim0: 100, 200, 300
-- enum ids for salaryFull.param_value: 22, 33
--
CREATE TABLE ageSex_p_2012817 
(
  run_id      INT   NOT NULL,
  dim0        INT   NOT NULL, 
  dim1        INT   NOT NULL, 
  param_value FLOAT NOT NULL,
  PRIMARY KEY (run_id, dim0, dim1)
);

CREATE TABLE ageSex_w_2012817
(
  set_id      INT   NOT NULL,
  dim0        INT   NOT NULL, 
  dim1        INT   NOT NULL, 
  param_value FLOAT NOT NULL,
  PRIMARY KEY (set_id, dim0, dim1)
);

CREATE TABLE salaryAge_p_2012818
(
  run_id      INT NOT NULL,
  dim0        INT NOT NULL, 
  dim1        INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (run_id, dim0, dim1)
);

CREATE TABLE salaryAge_w_2012818
(
  set_id      INT NOT NULL,
  dim0        INT NOT NULL, 
  dim1        INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (set_id, dim0, dim1)
);

CREATE TABLE StartingSeed_p_2012819
(
  run_id      INT NOT NULL,
  param_value INT NOT NULL,
  PRIMARY KEY (run_id)
);

CREATE TABLE StartingSeed_w_2012819
(
  set_id      INT NOT NULL,
  param_value INT NOT NULL,
  PRIMARY KEY (set_id)
);

CREATE TABLE salaryFull_p_2012812
(
  run_id      INT NOT NULL,
  dim0        INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (run_id, dim0)
);

CREATE TABLE salaryFull_w_2012812
(
  set_id      INT NOT NULL,
  dim0        INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (set_id, dim0)
);

--
-- modelOne output tables
-- enum ids for salarySex.dim0: 100, 200, 300
-- enum ids for salarySex.dim1: 0,   1
--
CREATE TABLE salarySex_a_2012820
(
  run_id    INT   NOT NULL,
  acc_id    INT   NOT NULL, 
  sub_id    INT   NOT NULL, 
  dim0      INT   NOT NULL, 
  dim1      INT   NOT NULL, 
  acc_value FLOAT NULL,
  PRIMARY KEY (run_id, acc_id, sub_id, dim0, dim1)
);

CREATE TABLE salarySex_v_2012820
(
  run_id     INT   NOT NULL,
  expr_id    INT   NOT NULL, 
  dim0       INT   NOT NULL, 
  dim1       INT   NOT NULL, 
  expr_value FLOAT NULL,
  PRIMARY KEY (run_id, expr_id, dim0, dim1)
);

--
-- modelOne all accumulators view
-- it does include all "native" accumulators: acc0, acc1
-- and "derived" accumulator: acc2 = acc0 + acc1
--
-- uncomment GO if MSSQL return an error on CREATE VIEW
-- GO
--
CREATE VIEW salarySex_d_2012820
AS
SELECT
  A.run_id,
  A.sub_id,
  A.dim0,
  A.dim1,
  acc0,
  acc1,
  (acc0 + acc1) AS acc2
FROM salarySex_a_2012820 A
INNER JOIN
(
  SELECT run_id, sub_id, dim0, dim1, acc_value AS acc0
  FROM salarySex_a_2012820
  WHERE acc_id = 0
) B0
ON (B0.run_id = A.run_id AND B0.sub_id = A.sub_id AND B0.dim0 = A.dim0 AND B0.dim1 = A.dim1)
INNER JOIN
(
  SELECT run_id, sub_id, dim0, dim1, acc_value AS acc1
  FROM salarySex_a_2012820
  WHERE acc_id = 1
) B1
ON (B1.run_id = A.run_id AND B1.sub_id = A.sub_id AND B1.dim0 = A.dim0 AND B1.dim1 = A.dim1)
WHERE A.acc_id = 0;
