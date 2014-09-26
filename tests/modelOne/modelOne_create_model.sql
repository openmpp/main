--
-- Copyright (c) 2014 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--

--
-- list of ids: values < 10 reserved for development and testing
-- test: model id = 1 is modelOne
-- test: set id = 2 is default set of parameters for modelOne
-- test: set id = 3 is modified working set for modelOne
--
-- INSERT INTO id_lst (id_key, id_value) VALUES ('model_id', 10);
-- INSERT INTO id_lst (id_key, id_value) VALUES ('run_id_set_id', 10);

--
-- modelOne metadata: describe the model
--
INSERT INTO model_dic
  (model_id, model_name, model_type, model_ver, model_ts, model_prefix, parameter_prefix, workset_prefix, acc_prefix, acc_flat_prefix, value_prefix)
VALUES
  (1, 'modelOne', 0, '0.1', '_201208171604590148_', 'modelOne_201208171604590148_', 'p', 'w', 'a', 'f', 'v');

INSERT INTO model_dic_txt (model_id, lang_id, descr, note) VALUES (1, 0, 'First model', NULL);
INSERT INTO model_dic_txt (model_id, lang_id, descr, note) VALUES (1, 1, 'First model (fr)', NULL);

-- 
-- modelOne default profile: default run options
--
INSERT INTO profile_lst (profile_name) VALUES ('modelOne');

INSERT INTO profile_option (profile_name, option_key, option_value) VALUES ('modelOne', 'Parameter.StartingSeed', '4095');
INSERT INTO profile_option (profile_name, option_key, option_value) VALUES ('modelOne', 'OpenM.SparseOutput', 'true');
-- INSERT INTO profile_option (profile_name, option_key, option_value) VALUES ('modelOne', 'OpenM.SparseNullValue', '');
-- INSERT INTO profile_option (profile_name, option_key, option_value) VALUES ('modelOne', 'General.Subsamples', '1');

-- 
-- modelOne simple types
--
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 0, 'char', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 1, 'schar', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 2, 'short', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 3, 'int', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 4, 'long', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 6, 'bool', 1, 2);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 5, 'llong', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 7, 'uchar', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 8, 'ushort', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 9, 'uint', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 10, 'ulong', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 11, 'ullong', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 12, 'float', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 13, 'double', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 14, 'ldouble', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 15, 'Time', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 16, 'real', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 17, 'integer', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 18, 'counter', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 19, 'file', 0, 1);

-- 
-- modelOne logical type
--
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 6, 'bool', 1, 2);

INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 6, 0, 'logical type', NULL);
INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 6, 1, 'logical type [no label (FR)]', NULL);

INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 6, 0, 'FALSE');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 6, 1, 'TRUE');

INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 6, 0, 0, 'False', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 6, 0, 1, 'Faux', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 6, 1, 0, 'True', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 6, 1, 1, 'Vrai', NULL);

-- 
-- modelOne classifcation types
--
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 21, 'age', 2, 4);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 22, 'sex', 2, 2);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 23, 'salary', 2, 3);

INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 21, 0, 'Age', NULL);
INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 21, 1, 'Age (fr)', NULL);
INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 22, 0, 'Sex', NULL);
INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 23, 0, 'Salary', NULL);

INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 21, 0, '10');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 21, 1, '20');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 21, 2, '30');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 21, 3, '40');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 22, 0, 'M');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 22, 1, 'F');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 23, 0, 'L');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 23, 1, 'M');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 23, 2, 'H');

INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 0, 0, 'age 10', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 0, 1, 'age 10 (fr)', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 1, 0, 'age 20', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 1, 1, 'age 20 (fr)', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 2, 0, 'age 30', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 2, 1, 'age 30 (fr)', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 3, 0, 'age 40', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 3, 1, 'age 40 (fr)', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 22, 0, 0, 'Male', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 22, 1, 0, 'Female', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 23, 0, 0, 'Low', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 23, 1, 0, 'Medium', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 23, 2, 0, 'High', NULL);

-- 
-- modelOne input parameters
--
INSERT INTO parameter_dic
  (model_id, parameter_id, db_name_suffix, parameter_name, parameter_rank, mod_type_id, is_hidden, is_generated, num_cumulated)
VALUES
  (1, 0, '0_ageSex', 'ageSex', 2, 13, 0, 0, 0);
INSERT INTO parameter_dic
  (model_id, parameter_id, db_name_suffix, parameter_name, parameter_rank, mod_type_id, is_hidden, is_generated, num_cumulated)
VALUES
  (1, 1, '1_salaryAge', 'salaryAge', 2, 3, 0, 0, 0);
INSERT INTO parameter_dic
  (model_id, parameter_id, db_name_suffix, parameter_name, parameter_rank, mod_type_id, is_hidden, is_generated, num_cumulated)
VALUES
  (1, 2, '2_StartingSeed', 'StartingSeed', 0, 3, 0, 0, 0);

INSERT INTO parameter_dic_txt (model_id, parameter_id, lang_id, descr, note) VALUES (1, 0, 0, 'Age by Sex', 'Age by Sex note');
INSERT INTO parameter_dic_txt (model_id, parameter_id, lang_id, descr, note) VALUES (1, 0, 1, 'Age by Sex (fr)', NULL);
INSERT INTO parameter_dic_txt (model_id, parameter_id, lang_id, descr, note) VALUES (1, 1, 0, 'Salary by Age', 'Salary by Age note');
INSERT INTO parameter_dic_txt (model_id, parameter_id, lang_id, descr, note) VALUES (1, 1, 1, 'Salary by Age (fr)', 'Salary by Age note (fr)');
INSERT INTO parameter_dic_txt (model_id, parameter_id, lang_id, descr, note) VALUES (1, 2, 1, 'Starting Seed', 'Random numbers generator starting seed value');

INSERT INTO parameter_dims (model_id, parameter_id, dim_name, dim_pos, mod_type_id) VALUES (1, 0, 'dim0', 0, 21);
INSERT INTO parameter_dims (model_id, parameter_id, dim_name, dim_pos, mod_type_id) VALUES (1, 0, 'dim1', 1, 22);
INSERT INTO parameter_dims (model_id, parameter_id, dim_name, dim_pos, mod_type_id) VALUES (1, 1, 'dim0', 0, 23);
INSERT INTO parameter_dims (model_id, parameter_id, dim_name, dim_pos, mod_type_id) VALUES (1, 1, 'dim1', 1, 21);

-- 
-- modelOne output tables
--
INSERT INTO table_dic 
  (model_id, table_id, db_name_suffix, table_name, is_user, table_rank, is_sparse, is_hidden, unit_dim_pos) 
VALUES 
  (1, 0, '0_salarySex', 'salarySex', 0, 2, 1, 0, 1);

INSERT INTO table_dic_txt
  (model_id, table_id, lang_id, descr, note, unit_descr, unit_note)
VALUES
  (1, 0, 0, 'Salary by Sex', 'Salary by Sex notes', 'Salary Dim', NULL);
INSERT INTO table_dic_txt
  (model_id, table_id, lang_id, descr, note, unit_descr, unit_note)
VALUES
  (1, 0, 1, 'Salary by Sex (fr)', 'Salary by Sex notes (fr)', 'Salary Dim (fr)', NULL);

INSERT INTO table_dims (model_id, table_id, dim_name, dim_pos, mod_type_id, is_total, dim_size) VALUES (1, 0, 'dim0', 0, 23, 0, 3);
INSERT INTO table_dims (model_id, table_id, dim_name, dim_pos, mod_type_id, is_total, dim_size) VALUES (1, 0, 'dim1', 1, 22, 0, 2);

INSERT INTO table_dims_txt (model_id, table_id, dim_name, lang_id, descr, note) VALUES (1, 0, 'dim0', 0, 'Salary Dim', 'Salary Dim notes');
INSERT INTO table_dims_txt (model_id, table_id, dim_name, lang_id, descr, note) VALUES (1, 0, 'dim0', 1, 'Salary Dim (fr)', 'Salary Dim notes (fr)');
INSERT INTO table_dims_txt (model_id, table_id, dim_name, lang_id, descr, note) VALUES (1, 0, 'dim1', 0, 'Sex Dim', 'Sex Dim notes');
INSERT INTO table_dims_txt (model_id, table_id, dim_name, lang_id, descr, note) VALUES (1, 0, 'dim1', 1, 'Sex Dim (fr)', NULL);

INSERT INTO table_acc (model_id, table_id, acc_id, acc_name, acc_expr) VALUES (1, 0, 0, 'acc0', 'value_sum()');
INSERT INTO table_acc (model_id, table_id, acc_id, acc_name, acc_expr) VALUES (1, 0, 1, 'acc1', 'value_count()');

INSERT INTO table_acc_txt (model_id, table_id, acc_id, lang_id, descr, note) VALUES (1, 0, 0, 0, 'Sum of salary by sex', NULL);
INSERT INTO table_acc_txt (model_id, table_id, acc_id, lang_id, descr, note) VALUES (1, 0, 1, 0, 'Count of salary by sex', NULL);

INSERT INTO table_unit 
  (model_id, table_id, unit_id, unit_name, unit_decimals, unit_src, unit_expr) 
VALUES 
  (1, 0, 0, 'expr0', 4, 'OM_AVG(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, AVG(M1.acc0) AS expr0 FROM modelOne_201208171604590148_f0_salarySex M1 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
INSERT INTO table_unit 
  (model_id, table_id, unit_id, unit_name, unit_decimals, unit_src, unit_expr) 
VALUES 
  (1, 0, 1, 'expr1', 4, 'OM_SUM(acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, AVG(M1.acc1) AS expr1 FROM modelOne_201208171604590148_f0_salarySex M1 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
INSERT INTO table_unit 
  (model_id, table_id, unit_id, unit_name, unit_decimals, unit_src, unit_expr) 
VALUES 
  (1, 0, 2, 'expr2', 2, 'OM_MIN(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, MIN(M1.acc0) AS expr2 FROM modelOne_201208171604590148_f0_salarySex M1 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
INSERT INTO table_unit 
  (model_id, table_id, unit_id, unit_name, unit_decimals, unit_src, unit_expr) 
VALUES 
  (1, 0, 3, 'expr3', 3, 'OM_AVG(acc0 * acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, AVG(M1.acc0 * M1.acc1) AS expr3 FROM modelOne_201208171604590148_f0_salarySex M1 GROUP BY M1.run_id, M1.dim0, M1.dim1'
);
  
INSERT INTO table_unit_txt (model_id, table_id, unit_id, lang_id, descr, note) VALUES (1, 0, 0, 0, 'Average acc0', 'Average on acc0 notes');
INSERT INTO table_unit_txt (model_id, table_id, unit_id, lang_id, descr, note) VALUES (1, 0, 0, 1, 'Average acc0 (fr)', 'Average on acc0 notes (fr)');
INSERT INTO table_unit_txt (model_id, table_id, unit_id, lang_id, descr, note) VALUES (1, 0, 1, 0, 'Sum acc1', NULL);
INSERT INTO table_unit_txt (model_id, table_id, unit_id, lang_id, descr, note) VALUES (1, 0, 2, 0, 'Min acc0', NULL);
INSERT INTO table_unit_txt (model_id, table_id, unit_id, lang_id, descr, note) VALUES (1, 0, 3, 0, 'Average acc0 * acc1', NULL);

--
-- modelOne input parameters
--
CREATE TABLE modelOne_201208171604590148_p0_ageSex 
(
  run_id INT   NOT NULL,
  dim0   INT   NOT NULL, 
  dim1   INT   NOT NULL, 
  value  FLOAT NOT NULL,
  PRIMARY KEY (run_id, dim0, dim1)
);

CREATE TABLE modelOne_201208171604590148_w0_ageSex
(
  set_id INT   NOT NULL,
  dim0   INT   NOT NULL, 
  dim1   INT   NOT NULL, 
  value  FLOAT NOT NULL,
  PRIMARY KEY (set_id, dim0, dim1)
);

CREATE TABLE modelOne_201208171604590148_p1_salaryAge 
(
  run_id INT NOT NULL,
  dim0   INT NOT NULL, 
  dim1   INT NOT NULL, 
  value  INT NOT NULL,
  PRIMARY KEY (run_id, dim0, dim1)
);

CREATE TABLE modelOne_201208171604590148_w1_salaryAge
(
  set_id INT NOT NULL,
  dim0   INT NOT NULL, 
  dim1   INT NOT NULL, 
  value  INT NOT NULL,
  PRIMARY KEY (set_id, dim0, dim1)
);

CREATE TABLE modelOne_201208171604590148_p2_StartingSeed 
(
  run_id INT NOT NULL,
  value  INT NOT NULL,
  PRIMARY KEY (run_id)
);

CREATE TABLE modelOne_201208171604590148_w2_StartingSeed
(
  set_id INT NOT NULL,
  value  INT NOT NULL,
  PRIMARY KEY (set_id)
);

--
-- modelOne output tables
--
CREATE TABLE modelOne_201208171604590148_a0_salarySex
(
  run_id    INT   NOT NULL,
  dim0      INT   NOT NULL, 
  dim1      INT   NOT NULL, 
  acc_id    INT   NOT NULL, 
  sub_id    INT   NOT NULL, 
  acc_value FLOAT NULL,
  PRIMARY KEY (run_id, dim0, dim1, acc_id, sub_id)
);

CREATE VIEW modelOne_201208171604590148_f0_salarySex
AS
SELECT 
  A0.run_id, A0.dim0, A0.dim1, A0.sub_id, A0.acc0, A1.acc1
FROM
(
  SELECT 
    run_id, dim0, dim1, sub_id, acc_value AS acc0
  FROM modelOne_201208171604590148_a0_salarySex 
  WHERE acc_id = 0
) A0
INNER JOIN
(
  SELECT 
    run_id, dim0, dim1, sub_id, acc_value AS acc1
  FROM modelOne_201208171604590148_a0_salarySex 
  WHERE acc_id = 1
) A1
ON (A1.run_id = A0.run_id AND A1.dim0 = A0.dim0 AND A1.dim1 = A0.dim1 AND A1.sub_id = A0.sub_id);

CREATE TABLE modelOne_201208171604590148_v0_salarySex
(
  run_id    INT   NOT NULL,
  dim0      INT   NOT NULL, 
  dim1      INT   NOT NULL, 
  unit_id   INT   NOT NULL, 
  value     FLOAT NULL,
  PRIMARY KEY (run_id, dim0, dim1, unit_id)
);

--
-- lines below is for information only and used by OpenM++
--

--
-- Simple types defined in Modgen-compatible way
--

-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 0,  'int', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 1,  'char', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 2,  'short', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 3,  'long', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 4,  'uint', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 5,  'uchar', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 6,  'ushort', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 7,  'ulong', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 8,  'integer', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 9,  'counter', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 10, 'real', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 11, 'float', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 12, 'double', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 13, 'TIME', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 14, 'rate', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 15, 'cumrate', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 16, 'haz1rate', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 17, 'haz2rate', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 18, 'piece_linear', 0, 1);
-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 19, 'file', 0, 1);

-- 
-- Logical type defined in Modgen-compatible way
--

-- INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 20, 'logical', 1, 2);
-- 
-- INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 20, 0, 'logical type', NULL);
-- INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 20, 1, 'logical type (fr)', NULL);
-- 
-- INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 20, 0, 'FALSE');
-- INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 20, 1, 'TRUE');
-- 
-- INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 20, 0, 0, 'False', NULL);
-- INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 20, 0, 1, 'Faux', NULL);
-- INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 20, 1, 0, 'True', NULL);
-- INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 20, 1, 1, 'Vrai', NULL);

