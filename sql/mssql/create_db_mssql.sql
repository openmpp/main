--
-- Copyright (c) 2014 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--

--
-- Id's source table
--
CREATE TABLE id_lst
(
  id_key   VARCHAR(32) NOT NULL, -- id key (based on table)
  id_value INT         NOT NULL, -- current id value
  PRIMARY KEY (id_key)
);

-- 
-- Language list
-- 
CREATE TABLE lang_lst
(
  lang_id   INT          NOT NULL, -- unique language id across all models
  lang_code VARCHAR(32)  NOT NULL, -- language code: en_US
  lang_name VARCHAR(255) NOT NULL, -- language name
  PRIMARY KEY (lang_id),
  CONSTRAINT lang_un UNIQUE (lang_code)
);

-- 
-- Language: list of keywords in specific language: all, min, max
-- 
CREATE TABLE lang_word
(
  lang_id    INT          NOT NULL, -- master key
  word_code  VARCHAR(255) NOT NULL, -- word key: all, min, max
  word_value VARCHAR(255) NOT NULL, -- actual word in that language
  PRIMARY KEY (lang_id, word_code),
  CONSTRAINT lang_word_mk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Model(s) list
--
CREATE TABLE model_dic
(
  model_id         INT          NOT NULL, -- unique model id for each model
  model_name       VARCHAR(255) NOT NULL, -- model name: modelOne
  model_digest     VARCHAR(32)  NOT NULL, -- model digest
  model_type       INT          NOT NULL, -- model type: 0 = case based, 1 = time based
  model_ver        VARCHAR(32)  NOT NULL, -- model version
  create_dt        VARCHAR(32)  NOT NULL, -- create date-time
  parameter_prefix VARCHAR(4)   NOT NULL, -- parameter tables prefix: _p
  workset_prefix   VARCHAR(4)   NOT NULL, -- workset tables prefix: _w
  acc_prefix       VARCHAR(4)   NOT NULL, -- accumulator tables prefix: _a
  value_prefix     VARCHAR(4)   NOT NULL, -- value tables prefix: _v
  PRIMARY KEY (model_id), 
  CONSTRAINT model_dic_un_1 UNIQUE (model_digest)
);

--
-- Model(s) list text info
--
CREATE TABLE model_dic_txt 
(
  model_id INT             NOT NULL, -- master key
  lang_id  INT             NOT NULL, -- language id
  descr    VARCHAR(255)    NOT NULL, -- model description
  note     TEXT,                     -- model notes
  PRIMARY KEY (model_id, lang_id),
  CONSTRAINT model_dic_txt_mk 
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id),
  CONSTRAINT model_dic_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

-- 
-- Types for parameters and dimensions
-- 
CREATE TABLE type_dic
(
  type_hid      INT          NOT NULL, -- unique type id
  type_name     VARCHAR(255) NOT NULL, -- type name: int, double...
  type_digest   VARCHAR(32)  NOT NULL, -- type digest or type name for built-in types
  dic_id        INT          NOT NULL, -- dictionary id: 0=simple 1=logical 2=classification 3=range 4=partition 5=link
  total_enum_id INT          NOT NULL, -- if total enabled this is enum_id of "Total" item =max+1
  PRIMARY KEY (type_hid),
  CONSTRAINT type_dic_un UNIQUE (type_digest)
);

-- 
-- Model types: for parameters and dimensions for each model 
-- 
CREATE TABLE model_type_dic
(
  model_id      INT NOT NULL, -- master key
  model_type_id INT NOT NULL, -- model type id
  type_hid      INT NOT NULL, -- type id
  PRIMARY KEY (model_id, model_type_id),
  CONSTRAINT model_type_dic_un UNIQUE (model_id, type_hid),
  CONSTRAINT model_type_dic_mk 
             FOREIGN KEY (type_hid) REFERENCES type_dic (type_hid),
  CONSTRAINT model_type_dic_fk
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

-- 
-- Types text: description and notes for the type
-- 
CREATE TABLE type_dic_txt
(
  type_hid    INT             NOT NULL, -- master key
  lang_id     INT             NOT NULL, -- language id
  descr       VARCHAR(255)    NOT NULL, -- type description
  note        TEXT,                     -- type notes
  PRIMARY KEY (type_hid, lang_id),
  CONSTRAINT type_dic_txt_mk 
             FOREIGN KEY (type_hid) REFERENCES type_dic (type_hid),
  CONSTRAINT type_dic_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- List of values for enum types, i.e. enum values for classification dictionary
--
CREATE TABLE type_enum_lst
(
  type_hid    INT          NOT NULL, -- master key
  enum_id     INT          NOT NULL, -- unique enum id, zero-based
  enum_name   VARCHAR(255) NOT NULL, -- enum name, ie: "true", "Ontario", "[90, max["
  PRIMARY KEY (type_hid, enum_id),
  CONSTRAINT type_enum_lst_mk
             FOREIGN KEY (type_hid) REFERENCES type_dic (type_hid)
);

-- 
-- Enum value text: description and notes for type values
-- 
CREATE TABLE type_enum_txt
(
  type_hid    INT             NOT NULL, -- master key
  enum_id     INT             NOT NULL, -- master key
  lang_id     INT             NOT NULL, -- language
  descr       VARCHAR(255)    NOT NULL, -- value description
  note        TEXT,                     -- value notes
  PRIMARY KEY (type_hid, enum_id, lang_id),
  CONSTRAINT type_enum_txt_mk 
             FOREIGN KEY (type_hid) REFERENCES type_dic (type_hid),
  CONSTRAINT type_enum_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Parameters list
--
CREATE TABLE parameter_dic
(
  parameter_hid    INT          NOT NULL, -- unique parameter id
  parameter_name   VARCHAR(255) NOT NULL, -- parameter name
  parameter_digest VARCHAR(32)  NOT NULL, -- parameter digest
  db_prefix        VARCHAR(32)  NOT NULL, -- parameter name part for db table name: ageSex
  db_suffix        VARCHAR(32)  NOT NULL, -- unique part for db table name: 12345678
  parameter_rank   INT          NOT NULL, -- parameter rank
  type_hid         INT          NOT NULL, -- parameter type id
  num_cumulated    INT          NOT NULL, -- number of cumulated dimensions
  PRIMARY KEY (parameter_hid),
  CONSTRAINT parameter_dic_un_1 UNIQUE (db_prefix, db_suffix),
  CONSTRAINT parameter_dic_un_2 UNIQUE (parameter_digest),
  CONSTRAINT parameter_dic_type_fk
             FOREIGN KEY (type_hid) REFERENCES type_dic (type_hid)
);

--
-- Model parameters
--
CREATE TABLE model_parameter_dic
(
  model_id           INT      NOT NULL, -- master key
  model_parameter_id INT      NOT NULL, -- model parameter id
  parameter_hid      INT      NOT NULL, -- unique parameter id
  is_hidden          SMALLINT NOT NULL, -- if <> 0 then parameter is hidden
  PRIMARY KEY (model_id, model_parameter_id),
  CONSTRAINT model_parameter_dic_un UNIQUE (model_id, parameter_hid),
  CONSTRAINT model_parameter_dic_mk 
             FOREIGN KEY (parameter_hid) REFERENCES parameter_dic (parameter_hid),
  CONSTRAINT model_parameter_dic_fk 
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

--
-- Parameter text info
--
CREATE TABLE parameter_dic_txt
(
  parameter_hid INT             NOT NULL, -- master key
  lang_id       INT             NOT NULL, -- language id
  descr         VARCHAR(255)    NOT NULL, -- parameter description
  note          TEXT,                     -- parameter notes
  PRIMARY KEY (parameter_hid, lang_id),
  CONSTRAINT parameter_dic_txt_mk
             FOREIGN KEY (parameter_hid) REFERENCES parameter_dic (parameter_hid),
  CONSTRAINT parameter_dic_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Parameter dimension(s)
--
CREATE TABLE parameter_dims
(
  parameter_hid INT        NOT NULL, -- master key
  dim_id        INT        NOT NULL, -- dimension index
  dim_name      VARCHAR(8) NOT NULL, -- column name: dim0
  type_hid      INT        NOT NULL, -- dimension type id
  PRIMARY KEY (parameter_hid, dim_id),
  CONSTRAINT parameter_dims_un UNIQUE (parameter_hid, dim_name),
  CONSTRAINT parameter_dims_mk
             FOREIGN KEY (parameter_hid) REFERENCES parameter_dic (parameter_hid),
  CONSTRAINT parameter_dims_type_fk
             FOREIGN KEY (type_hid) REFERENCES type_dic (type_hid)
);

--
-- Dimensions text info for parameter
--
CREATE TABLE parameter_dims_txt 
(
  parameter_hid INT             NOT NULL, -- master key
  dim_id        INT             NOT NULL, -- master key
  lang_id       INT             NOT NULL, -- language id
  descr         VARCHAR(255)    NOT NULL, -- table dimension description
  note          TEXT,                     -- table dimension notes
  PRIMARY KEY (parameter_hid, dim_id, lang_id),
  CONSTRAINT parameter_dims_txt_mk 
             FOREIGN KEY (parameter_hid, dim_id) REFERENCES parameter_dims (parameter_hid, dim_id),
  CONSTRAINT parameter_dims_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Output result tables
--
CREATE TABLE table_dic
(
  table_hid    INT          NOT NULL, -- unique table id
  table_name   VARCHAR(255) NOT NULL, -- table name
  table_digest VARCHAR(32)  NOT NULL, -- output table digest
  db_prefix    VARCHAR(32)  NOT NULL, -- name part for db table name: salaryBySex
  db_suffix    VARCHAR(32)  NOT NULL, -- unique part for db table name: 12345678
  table_rank   INT          NOT NULL, -- table rank
  is_sparse    SMALLINT     NOT NULL, -- if <> 0 then table stored as sparse
  PRIMARY KEY (table_hid),
  CONSTRAINT table_dic_un_1 UNIQUE (db_prefix, db_suffix),
  CONSTRAINT table_dic_un_2 UNIQUE (table_digest)
);

--
-- Model output result tables
--
CREATE TABLE model_table_dic 
(
  model_id       INT      NOT NULL, -- master key
  model_table_id INT      NOT NULL, -- model table id
  table_hid      INT      NOT NULL, -- master key
  is_user        SMALLINT NOT NULL, -- if <> 0 then "user" table
  expr_dim_pos   INT      NOT NULL, -- table expressions dimension (analysis dimension) position
  PRIMARY KEY (model_id, model_table_id),
  CONSTRAINT model_table_dic_un UNIQUE (model_id, table_hid),
  CONSTRAINT model_table_dic_mk 
             FOREIGN KEY (table_hid) REFERENCES table_dic (table_hid),
  CONSTRAINT model_table_dic_fk 
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

--
-- Output result tables text info
--
CREATE TABLE table_dic_txt 
(
  table_hid  INT             NOT NULL, -- master key
  lang_id    INT             NOT NULL, -- language id
  descr      VARCHAR(255)    NOT NULL, -- table description
  note       TEXT,                     -- table notes
  expr_descr VARCHAR(255)    NOT NULL, -- table expressions (analysis dimension) description
  expr_note  TEXT,                     -- table expressions (analysis dimension) notes
  PRIMARY KEY (table_hid, lang_id),
  CONSTRAINT table_dic_txt_mk 
             FOREIGN KEY (table_hid) REFERENCES table_dic (table_hid),
  CONSTRAINT table_dic_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Dimensions for output tables
--
CREATE TABLE table_dims 
(
  table_hid INT        NOT NULL, -- master key
  dim_id    INT        NOT NULL, -- dimension index
  dim_name  VARCHAR(8) NOT NULL, -- unique column name of dimension: dim0
  type_hid  INT        NOT NULL, -- dimension type
  is_total  SMALLINT   NOT NULL, -- if <> 0 then dimension has "total" item
  dim_size  INT        NOT NULL, -- number of items, including "total" item
  PRIMARY KEY (table_hid, dim_id),
  CONSTRAINT table_dims_un UNIQUE (table_hid, dim_name),
  CONSTRAINT table_dims_mk 
             FOREIGN KEY (table_hid) REFERENCES table_dic (table_hid),
  CONSTRAINT table_dims_type_fk 
             FOREIGN KEY (type_hid) REFERENCES type_dic (type_hid)
);

--
-- Dimensions text info for output tables
--
CREATE TABLE table_dims_txt 
(
  table_hid INT             NOT NULL, -- master key
  dim_id    INT             NOT NULL, -- master key
  lang_id   INT             NOT NULL, -- language id
  descr     VARCHAR(255)    NOT NULL, -- table dimension description
  note      TEXT,                     -- table dimension notes
  PRIMARY KEY (table_hid, dim_id, lang_id),
  CONSTRAINT table_dims_txt_mk 
             FOREIGN KEY (table_hid, dim_id) REFERENCES table_dims (table_hid, dim_id),
  CONSTRAINT table_dims_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Output table accumulators
--
CREATE TABLE table_acc 
(
  table_hid INT          NOT NULL, -- master key
  acc_id    INT          NOT NULL, -- unique accumulator id
  acc_name  VARCHAR(8)   NOT NULL, -- unique accumulator name: acc2
  acc_expr  VARCHAR(255) NOT NULL, -- accumulator expression: min_value_out(duration())
  PRIMARY KEY (table_hid, acc_id),
  CONSTRAINT table_acc_un UNIQUE (table_hid, acc_name),
  CONSTRAINT table_acc_mk 
             FOREIGN KEY (table_hid) REFERENCES table_dic (table_hid)
);

--
-- Output table accumulators text info
--
CREATE TABLE table_acc_txt 
(
  table_hid INT          NOT NULL, -- master key
  acc_id    INT          NOT NULL, -- master key
  lang_id   INT          NOT NULL, -- language id
  descr     VARCHAR(255) NOT NULL, -- item description
  note      TEXT,                  -- item notes
  PRIMARY KEY (table_hid, acc_id, lang_id),
  CONSTRAINT table_acc_txt_mk 
             FOREIGN KEY (table_hid, acc_id) REFERENCES table_acc (table_hid, acc_id),
  CONSTRAINT table_acc_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Output table expressions (analysis dimension items)
--
CREATE TABLE table_expr
(
  table_hid     INT           NOT NULL, -- master key
  expr_id       INT           NOT NULL, -- unique item id
  expr_name     VARCHAR(8)    NOT NULL, -- item name: expr2
  expr_decimals INT           NOT NULL, -- number of decimals for that item
  expr_src      VARCHAR(255)  NOT NULL, -- source expression: OM_AVG(acc3/acc0)
  expr_sql      VARCHAR(2048) NOT NULL, -- db expression: AVG(S.acc3/S.acc0)
  PRIMARY KEY (table_hid, expr_id),
  CONSTRAINT table_expr_un UNIQUE (table_hid, expr_name),
  CONSTRAINT table_expr_mk 
             FOREIGN KEY (table_hid) REFERENCES table_dic (table_hid)
);

--
-- Output table expressions text info
--
CREATE TABLE table_expr_txt 
(
  table_hid INT          NOT NULL, -- master key
  expr_id   INT          NOT NULL, -- master key
  lang_id   INT          NOT NULL, -- language id
  descr     VARCHAR(255) NOT NULL, -- item description
  note      TEXT,                  -- item notes
  PRIMARY KEY (table_hid, expr_id, lang_id),
  CONSTRAINT table_expr_txt_mk 
             FOREIGN KEY (table_hid, expr_id) REFERENCES table_expr (table_hid, expr_id),
  CONSTRAINT table_expr_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Groups: parameter or output table groups
--
CREATE TABLE group_lst
(
  model_id     INT          NOT NULL, -- master key
  group_id     INT          NOT NULL, -- unique model group id
  is_parameter SMALLINT     NOT NULL, -- if <> 0 then parameter group else output table group
  group_name   VARCHAR(255) NOT NULL, -- group name
  is_hidden    SMALLINT     NOT NULL, -- if <> 0 then group is hidden
  PRIMARY KEY (model_id, group_id),
  CONSTRAINT group_lst_mk 
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

--
-- Group text info for parameter or output table groups
--
CREATE TABLE group_txt
(
  model_id INT             NOT NULL, -- master key
  group_id INT             NOT NULL, -- master key
  lang_id  INT             NOT NULL, -- language id
  descr    VARCHAR(255)    NOT NULL, -- group description
  note     TEXT,                     -- group notes
  PRIMARY KEY (model_id, group_id, lang_id),
  CONSTRAINT group_txt_mk 
             FOREIGN KEY (model_id, group_id) REFERENCES group_lst (model_id, group_id),
  CONSTRAINT parameter_group_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Groups parent-child: subgroup, parameter or output table
--
CREATE TABLE group_pc
(
  model_id       INT NOT NULL, -- master key
  group_id       INT NOT NULL, -- master key
  child_pos      INT NOT NULL, -- child position in group
  child_group_id INT NULL,     -- if not IS NULL then child group
  leaf_id        INT NULL,     -- if not IS NULL then parameter id or table id
  PRIMARY KEY (model_id, group_id, child_pos),
  CONSTRAINT group_pc_mk 
             FOREIGN KEY (model_id, group_id) REFERENCES group_lst (model_id, group_id)
);

--
-- Profile list
-- profile is a named group of (key, value) options
-- default model options has profile_name = model_name
--
CREATE TABLE profile_lst
(
  profile_name VARCHAR(255) NOT NULL, -- unique profile name
  PRIMARY KEY (profile_name)
);

--
-- Profile options, ie: ini-file options
--
CREATE TABLE profile_option
(
  profile_name VARCHAR(255)  NOT NULL, -- master key
  option_key   VARCHAR(255)  NOT NULL, -- section.key, ie: General.Subsamples
  option_value VARCHAR(2048) NOT NULL, -- option value
  PRIMARY KEY (profile_name, option_key),
  CONSTRAINT profile_option_mk 
             FOREIGN KEY (profile_name) REFERENCES profile_lst(profile_name)
);

--
-- Model run's list
-- Run id must be different from working set id (use id_lst to get it)
--
CREATE TABLE run_lst
(
  run_id        INT          NOT NULL, -- unique run id
  model_id      INT          NOT NULL, -- model id
  run_name      VARCHAR(255) NOT NULL, -- model run name
  sub_count     INT          NOT NULL, -- subsamples count
  sub_started   INT          NOT NULL, -- number of subsamples started
  sub_completed INT          NOT NULL, -- number of subsamples completed
  sub_restart   INT          NOT NULL, -- subsample to restart from
  create_dt     VARCHAR(32)  NOT NULL, -- start date-time
  status        VARCHAR(1)   NOT NULL, -- run status: i=init p=progress s=success x=exit e=error(failed)
  update_dt     VARCHAR(32)  NOT NULL, -- last update date-time
  PRIMARY KEY (run_id),
  CONSTRAINT run_lst_mk 
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

--
-- Model run's text results
--
CREATE TABLE run_txt
(
  run_id   INT             NOT NULL, -- master key
  lang_id  INT             NOT NULL, -- language id
  descr    VARCHAR(255)    NOT NULL, -- model run description
  note     TEXT,                     -- model run notes
  PRIMARY KEY (run_id, lang_id),
  CONSTRAINT run_txt_mk 
             FOREIGN KEY (run_id) REFERENCES run_lst(run_id),
  CONSTRAINT run_txt_lang 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Run options: options used to run the model
-- model options priority:
-- from command line, ini-file, profile_option or default values
--
CREATE TABLE run_option
(
  run_id       INT           NOT NULL, -- master key
  option_key   VARCHAR(255)  NOT NULL, -- section.key, ie: General.Subsamples
  option_value VARCHAR(2048) NOT NULL, -- option value
  PRIMARY KEY (run_id, option_key),
  CONSTRAINT run_option_mk 
             FOREIGN KEY (run_id) REFERENCES run_lst(run_id)
);

--
-- Parameter run value list
-- "base run id" is run id where parameter value must be selected from
--
CREATE TABLE run_parameter
(
  run_id        INT         NOT NULL, -- master key
  parameter_hid INT         NOT NULL, -- parameter unique id
  base_run_id   INT         NOT NULL, -- source run id to select parameter value
  run_digest    VARCHAR(32) NOT NULL, -- digest of parameter value for the run
  PRIMARY KEY (run_id, parameter_hid),
  CONSTRAINT run_parameter_mk 
             FOREIGN KEY (run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_parameter_base_fk 
             FOREIGN KEY (base_run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_parameter_fk
             FOREIGN KEY (parameter_hid) REFERENCES parameter_dic (parameter_hid)
);

--
-- Parameter run text: parameter run value notes
--
CREATE TABLE run_parameter_txt
(
  run_id        INT NOT NULL,   -- model run id
  parameter_hid INT NOT NULL,   -- master key
  lang_id       INT NOT NULL,   -- language id
  note          TEXT,           -- parameter value notes
  PRIMARY KEY (run_id, parameter_hid, lang_id),
  CONSTRAINT run_parameter_txt_mk 
             FOREIGN KEY (run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_parameter_txt_fk 
             FOREIGN KEY (parameter_hid) REFERENCES parameter_dic (parameter_hid),
  CONSTRAINT run_parameter_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Output table run value list
-- "base run id" is run id where values of output table accumulators and expressions must be selected from
--
CREATE TABLE run_table
(
  run_id      INT         NOT NULL, -- master key
  table_hid   INT         NOT NULL, -- output table unique id
  base_run_id INT         NOT NULL, -- source run id to select output table value
  run_digest  VARCHAR(32) NOT NULL, -- digest of table value for the run
  PRIMARY KEY (run_id, table_hid),
  CONSTRAINT run_table_mk 
             FOREIGN KEY (run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_table_base_fk 
             FOREIGN KEY (base_run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_table_fk
             FOREIGN KEY (table_hid) REFERENCES table_dic (table_hid)
);

--
-- Workset (working set of model input parameters):
--   it can be a full set, which include all model parameters 
--   or subset and include only some parameters
-- each model must have "default" workset:
--   default workset must include ALL model parameters (it is a full set)
--   default workset is a first workset of the model: set_id = min(set_id)
-- if workset is a subset (does not include all model parameters)
--   then it must be based on model run results, specified by run_id (not NULL)
-- workset can be editable or read-only
--   if workset is editable then you can modify input parameters or workset description, notes, etc.
--   if workset is read-only then you can run the model using that workset as input
-- Important: working set_id must be different from run_id (use id_lst to get it)
-- Important: always update parameter values inside of transaction scope
-- Important: before parameter update do is_readonly = is_readonly + 1 to "lock" workset
--
CREATE TABLE workset_lst
(
  set_id      INT          NOT NULL, -- unique working set id
  base_run_id INT,                   -- if not NULL and positive then base run id (source run id)
  model_id    INT          NOT NULL, -- model id
  set_name    VARCHAR(255) NOT NULL, -- working set name
  is_readonly SMALLINT     NOT NULL, -- if non-zero then working set is read-only
  update_dt   VARCHAR(32)  NOT NULL, -- last update date-time
  PRIMARY KEY (set_id),
  CONSTRAINT workset_lst_un_1 UNIQUE (model_id, set_name),
  CONSTRAINT workset_lst_mk
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id),
  CONSTRAINT workset_lst_fk
             FOREIGN KEY (base_run_id) REFERENCES run_lst (run_id)
);

--
-- Working set text: description and notes
--
CREATE TABLE workset_txt
(
  set_id   INT             NOT NULL, -- master key
  lang_id  INT             NOT NULL, -- language id
  descr    VARCHAR(255)    NOT NULL, -- working set description
  note     TEXT,                     -- working set notes
  PRIMARY KEY (set_id, lang_id),
  CONSTRAINT workset_txt_mk 
             FOREIGN KEY (set_id) REFERENCES workset_lst (set_id),
  CONSTRAINT workset_txt_lang 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Working set parameters list
--
CREATE TABLE workset_parameter
(
  set_id        INT NOT NULL, -- master key
  parameter_hid INT NOT NULL, -- parameter_dic.parameter_id
  PRIMARY KEY (set_id, parameter_hid),
  CONSTRAINT workset_parameter_mk 
             FOREIGN KEY (set_id) REFERENCES workset_lst (set_id),
  CONSTRAINT workset_parameter_param_fk
             FOREIGN KEY (parameter_hid) REFERENCES parameter_dic (parameter_hid)
);

--
-- Working set parameter text: parameter value notes
--
CREATE TABLE workset_parameter_txt
(
  set_id        INT             NOT NULL, -- master key
  parameter_hid INT             NOT NULL, -- master key
  lang_id       INT             NOT NULL, -- language id
  note          TEXT,                     -- parameter value note
  PRIMARY KEY (set_id, parameter_hid, lang_id),
  CONSTRAINT workset_parameter_txt_mk 
             FOREIGN KEY (set_id, parameter_hid) REFERENCES workset_parameter (set_id, parameter_hid),
  CONSTRAINT workset_parameter_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Modelling task: named set of model inputs (of working sets)
--
CREATE TABLE task_lst
(
  task_id   INT          NOT NULL, -- unique task id
  model_id  INT          NOT NULL, -- master key
  task_name VARCHAR(255) NOT NULL, -- task name
  PRIMARY KEY (task_id),
  CONSTRAINT task_lst_mk 
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

--
-- Modelling task text: description and notes
--
CREATE TABLE task_txt
(
  task_id INT             NOT NULL, -- master key
  lang_id INT             NOT NULL, -- language id
  descr   VARCHAR(255)    NOT NULL, -- task description
  note    TEXT,                     -- task notes
  PRIMARY KEY (task_id, lang_id),
  CONSTRAINT task_txt_mk 
             FOREIGN KEY (task_id) REFERENCES task_lst(task_id),
  CONSTRAINT task_txt_lang 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Task input: working sets of model input parameters
--
CREATE TABLE task_set
(
  task_id INT NOT NULL, -- master key
  set_id  INT NOT NULL, -- input working set id
  PRIMARY KEY (task_id, set_id),
  CONSTRAINT task_set_mk 
             FOREIGN KEY (task_id) REFERENCES task_lst (task_id),
  CONSTRAINT task_set_fk 
             FOREIGN KEY (set_id) REFERENCES workset_lst (set_id)
);

--
-- Task run history and status
-- if status = w (wait) then 
--    model wait and NOT completed until other process set status to one of finals: s,x,e
--    model check if any new sets inserted into task_set and run it as they arrive
--
CREATE TABLE task_run_lst
(
  task_run_id INT         NOT NULL, -- unique task run id
  task_id     INT         NOT NULL, -- master key
  sub_count   INT         NOT NULL, -- subsamples count of task run
  create_dt   VARCHAR(32) NOT NULL, -- start date-time
  status      VARCHAR(1)  NOT NULL, -- task status: i=init p=progress w=wait s=success x=exit e=error(failed)
  update_dt   VARCHAR(32) NOT NULL, -- last update date-time
  PRIMARY KEY (task_run_id),
  CONSTRAINT task_run_lst_mk 
             FOREIGN KEY (task_id) REFERENCES task_lst (task_id)
);

--
-- Task run: input (working sets) and output (model run)
--
CREATE TABLE task_run_set
(
  task_run_id INT NOT NULL, -- master key
  run_id      INT NOT NULL, -- if > 0 then result run id
  set_id      INT NOT NULL, -- if > 0 then input working set id
  task_id     INT NOT NULL, -- link to task list
  PRIMARY KEY (task_run_id, run_id),
  CONSTRAINT task_rs_un 
             UNIQUE (task_run_id, set_id),
  CONSTRAINT task_rs_mk 
             FOREIGN KEY (task_run_id) REFERENCES task_run_lst (task_run_id)
);

--
-- list of ids, must be positive.
-- values < 10 reserved for development and testing
--
INSERT INTO id_lst (id_key, id_value) VALUES ('openmpp',       10);
INSERT INTO id_lst (id_key, id_value) VALUES ('lang_id',       10);
INSERT INTO id_lst (id_key, id_value) VALUES ('model_id',      10);
INSERT INTO id_lst (id_key, id_value) VALUES ('type_hid',      30);
INSERT INTO id_lst (id_key, id_value) VALUES ('parameter_hid', 10);
INSERT INTO id_lst (id_key, id_value) VALUES ('table_hid',     10);
INSERT INTO id_lst (id_key, id_value) VALUES ('run_id_set_id', 10);
INSERT INTO id_lst (id_key, id_value) VALUES ('task_id',       10);
INSERT INTO id_lst (id_key, id_value) VALUES ('task_run_id',   10);

--
-- Languages and word list
--
INSERT INTO lang_lst (lang_id, lang_code, lang_name) VALUES (0, 'EN', 'English');
INSERT INTO lang_lst (lang_id, lang_code, lang_name) VALUES (1, 'FR', 'Français');

INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'all', 'All');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'min', 'min');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'max', 'max');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'all', 'Toutes');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'min', 'min');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'max', 'max');

--
-- built-in types: type name used as unique type digest
--
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (0, 'char',  '_char_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (1, 'schar', '_schar_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (2, 'short', '_short_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (3, 'int',   '_int_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (4, 'long',  '_long_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (5, 'llong', '_llong_', 0, 1);
-- INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (6, 'bool',  '_bool_', 1, 2);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (7, 'uchar',    '_uchar_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (8, 'ushort',   '_ushort_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (9, 'uint',     '_uint_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (10, 'ulong',   '_ulong_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (11, 'ullong',  '_ullong_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (12, 'float',   '_float_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (13, 'double',  '_double_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (14, 'ldouble', '_ldouble_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (15, 'Time',    '_time_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (16, 'real',    '_real_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (17, 'integer', '_integer_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (18, 'counter', '_counter_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (19, 'big_counter', '_big_counter_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (20, 'file',    '_file_', 0, 1);

-- 
-- built-in types: logical type
--
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (6, 'bool', '_bool_', 1, 2);

INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (6, 0, 'logical type', NULL);
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (6, 1, 'logical type [no label (FR)]', NULL);

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (6, 0, 'false');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (6, 1, 'true');

INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (6, 0, 0, 'False', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (6, 0, 1, 'Faux', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (6, 1, 0, 'True', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (6, 1, 1, 'Vrai', NULL);
