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
  lang_code VARCHAR(32)  NOT NULL, -- language code: EN, FR, en-CA
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
  model_id        INT          NOT NULL, -- unique model id for each model
  model_name      VARCHAR(255) NOT NULL, -- model name: modelOne
  model_digest    VARCHAR(32)  NOT NULL, -- model digest
  model_type      INT          NOT NULL, -- model type: 0 = case based, 1 = time based
  model_ver       VARCHAR(32)  NOT NULL, -- model version
  create_dt       VARCHAR(32)  NOT NULL, -- create date-time
  default_lang_id INT          NOT NULL, -- model default language
  PRIMARY KEY (model_id),
  CONSTRAINT model_dic_un UNIQUE (model_digest),
  CONSTRAINT model_dic_lang_fk
             FOREIGN KEY (default_lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Model(s) list text info
--
CREATE TABLE model_dic_txt
(
  model_id INT             NOT NULL, -- master key
  lang_id  INT             NOT NULL, -- language id
  descr    VARCHAR(255)    NOT NULL, -- model description
  note     CLOB,                     -- model notes
  PRIMARY KEY (model_id, lang_id),
  CONSTRAINT model_dic_txt_mk
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id),
  CONSTRAINT model_dic_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Model language-specific messages (translated strings)
--
CREATE TABLE model_word
(
  model_id   INT          NOT NULL, -- master key
  lang_id    INT          NOT NULL, -- language id
  word_code  VARCHAR(255) NOT NULL, -- word key: all, min, max
  word_value VARCHAR(255) NULL,     -- if not NULL then actual word in that language
  PRIMARY KEY (model_id, lang_id, word_code),
  CONSTRAINT model_word_mk
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id),
  CONSTRAINT model_word_fk
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
  note        CLOB,                     -- type notes
  PRIMARY KEY (type_hid, lang_id),
  CONSTRAINT type_dic_txt_mk
             FOREIGN KEY (type_hid) REFERENCES type_dic (type_hid),
  CONSTRAINT type_dic_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- List of values for enum types, i.e. enum values for classification dictionary
-- Enum names must be unique, even no such constraint defined
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
  note        CLOB,                     -- value notes
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
  parameter_rank   INT          NOT NULL, -- parameter rank
  type_hid         INT          NOT NULL, -- parameter type id
  is_extendable    SMALLINT     NOT NULL, -- if non-zero then parameter value can be NULL
  num_cumulated    INT          NOT NULL, -- number of cumulated dimensions
  db_run_table     VARCHAR(64)  NOT NULL, -- parameter run values db table name: ageSex_p12345678
  db_set_table     VARCHAR(64)  NOT NULL, -- parameter workset values db table name: ageSex_w12345678
  import_digest    VARCHAR(32)  NOT NULL, -- import digest to link parameters between models
  PRIMARY KEY (parameter_hid),
  CONSTRAINT parameter_dic_un UNIQUE (parameter_digest),
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
  is_hidden          SMALLINT NOT NULL, -- if non-zero then parameter is hidden
  PRIMARY KEY (model_id, model_parameter_id),
  CONSTRAINT model_parameter_dic_un UNIQUE (model_id, parameter_hid),
  CONSTRAINT model_parameter_dic_mk
             FOREIGN KEY (parameter_hid) REFERENCES parameter_dic (parameter_hid),
  CONSTRAINT model_parameter_dic_fk
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

--
-- Model parameters import: link parameter to upstream model output table or parameter
--
CREATE TABLE model_parameter_import
(
  model_id           INT          NOT NULL, -- master key
  model_parameter_id INT          NOT NULL, -- master key
  from_name          VARCHAR(255) NOT NULL, -- name of source output table or parameter
  from_model_name    VARCHAR(255) NOT NULL, -- source model name
  is_sample_dim      SMALLINT     NOT NULL, -- not used by openM++
  PRIMARY KEY (model_id, model_parameter_id),
  CONSTRAINT model_parameter_import_mk
             FOREIGN KEY (model_id, model_parameter_id) REFERENCES model_parameter_dic (model_id, model_parameter_id)
);

--
-- Parameter text info
--
CREATE TABLE parameter_dic_txt
(
  parameter_hid INT             NOT NULL, -- master key
  lang_id       INT             NOT NULL, -- language id
  descr         VARCHAR(255)    NOT NULL, -- parameter description
  note          CLOB,                     -- parameter notes
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
  parameter_hid INT          NOT NULL, -- master key
  dim_id        INT          NOT NULL, -- dimension index
  dim_name      VARCHAR(255) NOT NULL, -- dimension name: Year
  type_hid      INT          NOT NULL, -- dimension type id
  PRIMARY KEY (parameter_hid, dim_id),
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
  note          CLOB,                     -- table dimension notes
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
  table_hid       INT          NOT NULL, -- unique table id
  table_name      VARCHAR(255) NOT NULL, -- table name
  table_digest    VARCHAR(32)  NOT NULL, -- output table digest
  table_rank      INT          NOT NULL, -- table rank
  is_sparse       SMALLINT     NOT NULL, -- if non-zero then table stored as sparse
  db_expr_table   VARCHAR(64)  NOT NULL, -- run values db table name: salaryBySex_v12345678
  db_acc_table    VARCHAR(64)  NOT NULL, -- accumulator values db table name: salaryBySex_a12345678
  db_acc_all_view VARCHAR(64)  NOT NULL, -- db view of all accumulators, including derived: salaryBySex_d12345678
  import_digest   VARCHAR(32)  NOT NULL, -- import digest to link output table to parameter of other model
  PRIMARY KEY (table_hid),
  CONSTRAINT table_dic_un UNIQUE (table_digest)
);

--
-- Model output result tables
--
CREATE TABLE model_table_dic
(
  model_id       INT      NOT NULL, -- master key
  model_table_id INT      NOT NULL, -- model table id
  table_hid      INT      NOT NULL, -- master key
  is_user        SMALLINT NOT NULL, -- if non-zero then "user" table
  expr_dim_pos   INT      NOT NULL, -- table expressions dimension (analysis dimension) position
  is_hidden      SMALLINT NOT NULL, -- if non-zero then output table is hidden
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
  note       CLOB,                     -- table notes
  expr_descr VARCHAR(255)    NOT NULL, -- table expressions (analysis dimension) description
  expr_note  CLOB,                     -- table expressions (analysis dimension) notes
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
  table_hid   INT          NOT NULL, -- master key
  dim_id      INT          NOT NULL, -- dimension index
  dim_name    VARCHAR(255) NOT NULL, -- dimension name: Year
  type_hid    INT          NOT NULL, -- dimension type
  is_total    SMALLINT     NOT NULL, -- if non-zero then dimension has "total" item
  dim_size    INT          NOT NULL, -- number of items, including "total" item
  PRIMARY KEY (table_hid, dim_id),
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
  note      CLOB,                     -- table dimension notes
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
  table_hid  INT           NOT NULL, -- master key
  acc_id     INT           NOT NULL, -- unique accumulator id
  acc_name   VARCHAR(255)  NOT NULL, -- unique accumulator name: acc2
  is_derived SMALLINT      NOT NULL, -- if non-zero then accumulator is expression on other accumulators
  acc_src    VARCHAR(255)  NOT NULL, -- source expression: acc0 + acc1
  acc_sql    VARCHAR(2048) NOT NULL, -- db expression: sql subquery
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
  descr     VARCHAR(255) NOT NULL, -- accumulator description
  note      CLOB,                  -- accumulator notes
  PRIMARY KEY (table_hid, acc_id, lang_id),
  CONSTRAINT table_acc_txt_mk
             FOREIGN KEY (table_hid, acc_id) REFERENCES table_acc (table_hid, acc_id),
  CONSTRAINT table_acc_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Output table expressions (measures)
--
CREATE TABLE table_expr
(
  table_hid     INT           NOT NULL, -- master key
  expr_id       INT           NOT NULL, -- unique measure id
  expr_name     VARCHAR(255)  NOT NULL, -- unique measure name: expr2
  expr_decimals INT           NOT NULL, -- number of decimals for that measure
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
  descr     VARCHAR(255) NOT NULL, -- expression description
  note      CLOB,                  -- expression notes
  PRIMARY KEY (table_hid, expr_id, lang_id),
  CONSTRAINT table_expr_txt_mk
             FOREIGN KEY (table_hid, expr_id) REFERENCES table_expr (table_hid, expr_id),
  CONSTRAINT table_expr_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Entities
--
CREATE TABLE entity_dic
(
  entity_hid    INT          NOT NULL, -- unique entity id
  entity_name   VARCHAR(255) NOT NULL, -- entity name
  entity_digest VARCHAR(32)  NOT NULL, -- entity digest
  PRIMARY KEY (entity_hid),
  CONSTRAINT entity_dic_un UNIQUE (entity_digest)
);

--
-- Model entities
--
CREATE TABLE model_entity_dic
(
  model_id        INT      NOT NULL, -- master key
  model_entity_id INT      NOT NULL, -- model entity id
  entity_hid      INT      NOT NULL, -- master key
  PRIMARY KEY (model_id, model_entity_id),
  CONSTRAINT model_entity_dic_un UNIQUE (model_id, entity_hid),
  CONSTRAINT model_entity_dic_mk
             FOREIGN KEY (entity_hid) REFERENCES entity_dic (entity_hid),
  CONSTRAINT model_entity_dic_fk
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

--
-- Entity text info: description and notes
--
CREATE TABLE entity_dic_txt
(
  entity_hid INT            NOT NULL, -- master key
  lang_id    INT            NOT NULL, -- language id
  descr      VARCHAR(255)   NOT NULL, -- entity description
  note       CLOB,                    -- entity notes
  PRIMARY KEY (entity_hid, lang_id),
  CONSTRAINT entity_dic_txt_mk
             FOREIGN KEY (entity_hid) REFERENCES entity_dic (entity_hid),
  CONSTRAINT entity_dic_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Entity attributes
--
CREATE TABLE entity_attr
(
  entity_hid  INT          NOT NULL, -- master key
  attr_id     INT          NOT NULL, -- attribute index
  attr_name   VARCHAR(255) NOT NULL, -- attribute name: Year
  type_hid    INT          NOT NULL, -- attribute type
  is_internal SMALLINT     NOT NULL, -- if non-zero then attribute is internal, e.g.: om_attr_year
  PRIMARY KEY (entity_hid, attr_id),
  CONSTRAINT entity_attr_mk
             FOREIGN KEY (entity_hid) REFERENCES entity_dic (entity_hid),
  CONSTRAINT entity_attr_type_fk
             FOREIGN KEY (type_hid) REFERENCES type_dic (type_hid)
);

--
-- Entity attributes text info: description and notes
--
CREATE TABLE entity_attr_txt
(
  entity_hid INT            NOT NULL, -- master key
  attr_id    INT            NOT NULL, -- master key
  lang_id    INT            NOT NULL, -- language id
  descr      VARCHAR(255)   NOT NULL, -- entity attribute description
  note       CLOB,                    -- entity attribute notes
  PRIMARY KEY (entity_hid, attr_id, lang_id),
  CONSTRAINT entity_attr_txt_mk
             FOREIGN KEY (entity_hid, attr_id) REFERENCES entity_attr (entity_hid, attr_id),
  CONSTRAINT entity_attr_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Groups: parameter or output table groups
--
CREATE TABLE group_lst
(
  model_id     INT          NOT NULL, -- master key
  group_id     INT          NOT NULL, -- unique model group id
  is_parameter SMALLINT     NOT NULL, -- if non-zero then parameter group else output table group
  group_name   VARCHAR(255) NOT NULL, -- group name
  is_hidden    SMALLINT     NOT NULL, -- if non-zero then group is hidden
  PRIMARY KEY (model_id, group_id),
  CONSTRAINT group_lst_un UNIQUE (model_id, group_name),
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
  note     CLOB,                     -- group notes
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
  profile_name VARCHAR(255)   NOT NULL, -- master key
  option_key   VARCHAR(255)   NOT NULL, -- section.key, ie: OpenM.Threads
  option_value CLOB           NOT NULL, -- option value
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
  sub_count     INT          NOT NULL, -- sub-value count
  sub_started   INT          NOT NULL, -- number of sub-values started
  sub_completed INT          NOT NULL, -- number of sub-values completed
  sub_restart   INT          NOT NULL, -- sub-value to restart from
  create_dt     VARCHAR(32)  NOT NULL, -- run created date-time
  status        VARCHAR(1)   NOT NULL, -- run status: i=init p=progress s=success x=exit e=error(failed)
  update_dt     VARCHAR(32)  NOT NULL, -- last update date-time
  run_digest    VARCHAR(32)  NOT NULL, -- digest of the run metadata: model digest, run name, sub count, created date-time, run stamp
  value_digest  VARCHAR(32),           -- if not NULL then digest of the run values: all parameters and output tables
  run_stamp     VARCHAR(32)  NOT NULL, -- process run stamp, by default is log time stamp
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
  note     CLOB,                     -- model run notes
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
  run_id       INT            NOT NULL, -- master key
  option_key   VARCHAR(255)   NOT NULL, -- section.key, ie: OpenM.Threads
  option_value CLOB           NOT NULL, -- option value
  PRIMARY KEY (run_id, option_key),
  CONSTRAINT run_option_mk
             FOREIGN KEY (run_id) REFERENCES run_lst(run_id)
);

--
-- Run parameters list
-- "base run id" is run id where parameter value must be selected from
--
CREATE TABLE run_parameter
(
  run_id        INT         NOT NULL, -- master key
  parameter_hid INT         NOT NULL, -- parameter unique id
  base_run_id   INT         NOT NULL, -- source run id to select parameter value
  sub_count     INT         NOT NULL, -- sub-values count
  value_digest  VARCHAR(32),          -- if not NULL then digest of parameter value for the run
  PRIMARY KEY (run_id, parameter_hid),
  CONSTRAINT run_parameter_mk
             FOREIGN KEY (run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_parameter_base_fk
             FOREIGN KEY (base_run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_parameter_fk
             FOREIGN KEY (parameter_hid) REFERENCES parameter_dic (parameter_hid)
);

--
-- Run parameters text: parameter run value notes
--
CREATE TABLE run_parameter_txt
(
  run_id        INT             NOT NULL, -- master key
  parameter_hid INT             NOT NULL, -- parameter unique id
  lang_id       INT             NOT NULL, -- language id
  note          CLOB,                     -- parameter value notes
  PRIMARY KEY (run_id, parameter_hid, lang_id),
  CONSTRAINT run_parameter_txt_mk
             FOREIGN KEY (run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_parameter_txt_fk
             FOREIGN KEY (parameter_hid) REFERENCES parameter_dic (parameter_hid),
  CONSTRAINT run_parameter_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Run parameters import: link parameter to upstream model output table or parameter
--
CREATE TABLE run_parameter_import
(
  run_id             INT          NOT NULL, -- master key
  parameter_hid      INT          NOT NULL, -- master key
  is_from_parameter  SMALLINT     NOT NULL, -- if non-zero then imported from parameter else from output table
  from_model_id      INT          NOT NULL, -- source model id
  from_model_digest  VARCHAR(32)  NOT NULL, -- source model digest
  from_run_id        INT          NOT NULL, -- source run id
  from_run_digest    VARCHAR(32)  NOT NULL, -- source run digest
  PRIMARY KEY (run_id, parameter_hid),
  CONSTRAINT run_parameter_import_mk
             FOREIGN KEY (run_id, parameter_hid) REFERENCES run_parameter (run_id, parameter_hid)
);

--
-- Output table run value list
-- "base run id" is run id where values of output table accumulators and expressions must be selected from
--
CREATE TABLE run_table
(
  run_id       INT         NOT NULL, -- master key
  table_hid    INT         NOT NULL, -- output table unique id
  base_run_id  INT         NOT NULL, -- source run id to select output table value
  value_digest VARCHAR(32),          -- if not NULL then digest of table value for the run
  PRIMARY KEY (run_id, table_hid),
  CONSTRAINT run_table_mk
             FOREIGN KEY (run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_table_base_fk
             FOREIGN KEY (base_run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_table_fk
             FOREIGN KEY (table_hid) REFERENCES table_dic (table_hid)
);

--
-- Run progress for each sub-value
-- progress_count usually contains percent of completed cases (case based models) or time (time based)
-- progress_value usually contains number cases completed (case based models) or time (time based)
--
CREATE TABLE run_progress
(
  run_id         INT         NOT NULL, -- master key
  sub_id         INT         NOT NULL, -- sub-value id (zero based index)
  create_dt      VARCHAR(32) NOT NULL, -- sub-value start date-time
  status         VARCHAR(1)  NOT NULL, -- run status: i=init p=progress s=success x=exit e=error(failed) d=delete
  update_dt      VARCHAR(32) NOT NULL, -- last update date-time
  progress_count INT         NOT NULL, -- progress count: percent completed
  progress_value FLOAT       NOT NULL, -- progress value: number of cases (case based) or time (time based)
  PRIMARY KEY (run_id, sub_id),
  CONSTRAINT run_progress_mk
             FOREIGN KEY (run_id) REFERENCES run_lst (run_id)
);

--
-- Entity generation:
-- partial instance of entity, may include only some of entity attributes
--
CREATE TABLE entity_gen
(
  entity_gen_hid  INT         NOT NULL, -- entity generation unique id
  entity_hid      INT         NOT NULL, -- master key
  db_entity_table VARCHAR(64) NOT NULL, -- db table name: Person_g87abcdef or Person_e87abcdef
  gen_digest      VARCHAR(32) NOT NULL, -- digest of entity generation
  PRIMARY KEY (entity_gen_hid),
  CONSTRAINT entity_gen_un UNIQUE (db_entity_table),
  CONSTRAINT entity_gen_fk
             FOREIGN KEY (entity_hid) REFERENCES entity_dic (entity_hid)
);

--
-- Entity generation attributes
--
CREATE TABLE entity_gen_attr
(
  entity_gen_hid INT NOT NULL, -- master key
  attr_id        INT NOT NULL, -- attribute index
  entity_hid     INT NOT NULL, -- entity unique id
  PRIMARY KEY (entity_gen_hid, attr_id),
  CONSTRAINT entity_gen_attr_mk
             FOREIGN KEY (entity_gen_hid) REFERENCES entity_gen (entity_gen_hid),
  CONSTRAINT entity_gen_attr_fk
             FOREIGN KEY (entity_hid, attr_id) REFERENCES entity_attr (entity_hid, attr_id)
);

--
-- Run entities list
-- "base run id" is run id where values of entity attribute values must be selected from
--
CREATE TABLE run_entity
(
  run_id         INT         NOT NULL, -- master key
  entity_gen_hid INT         NOT NULL, -- entity generation unique id
  base_run_id    INT         NOT NULL, -- source run id to select entity attribute values
  value_digest   VARCHAR(32),          -- if not NULL then digest of attribute values for the run
  PRIMARY KEY (run_id, entity_gen_hid),
  CONSTRAINT run_entity_mk
             FOREIGN KEY (run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_entity_base_fk
             FOREIGN KEY (base_run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_entity_gen_fk
             FOREIGN KEY (entity_gen_hid) REFERENCES entity_gen (entity_gen_hid)
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
-- workset name + model id must be unique (model cannot have multiple workset with same name)
-- working set_id must be different from run_id (use id_lst to get it)
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
  CONSTRAINT workset_lst_un UNIQUE (model_id, set_name),
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
  note     CLOB,                     -- working set notes
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
  set_id         INT NOT NULL, -- master key
  parameter_hid  INT NOT NULL, -- parameter_dic.parameter_id
  sub_count      INT NOT NULL, -- sub-values count
  default_sub_id INT NOT NULL, -- default sub-value id
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
  note          CLOB,                     -- parameter value note
  PRIMARY KEY (set_id, parameter_hid, lang_id),
  CONSTRAINT workset_parameter_txt_mk
             FOREIGN KEY (set_id, parameter_hid) REFERENCES workset_parameter (set_id, parameter_hid),
  CONSTRAINT workset_parameter_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Modelling task: named set of model inputs (of working sets)
-- task name + model id must be unique (model cannot have multiple tasks with same name)
--
CREATE TABLE task_lst
(
  task_id   INT          NOT NULL, -- unique task id
  model_id  INT          NOT NULL, -- master key
  task_name VARCHAR(255) NOT NULL, -- task name
  PRIMARY KEY (task_id),
  CONSTRAINT task_lst_un UNIQUE (model_id, task_name),
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
  note    CLOB,                     -- task notes
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
  task_run_id INT          NOT NULL, -- unique task run id
  task_id     INT          NOT NULL, -- master key
  run_name    VARCHAR(255) NOT NULL, -- task run name
  sub_count   INT          NOT NULL, -- sub-values count of task run
  create_dt   VARCHAR(32)  NOT NULL, -- start date-time
  status      VARCHAR(1)   NOT NULL, -- task status: i=init p=progress w=wait s=success x=exit e=error(failed)
  update_dt   VARCHAR(32)  NOT NULL, -- last update date-time
  run_stamp   VARCHAR(32)  NOT NULL, -- process run stamp, by default is log time stamp
  PRIMARY KEY (task_run_id),
  CONSTRAINT task_run_lst_mk
             FOREIGN KEY (task_id) REFERENCES task_lst (task_id)
);

--
-- Task run history: input (working sets) and output (model run)
--    working sets in task run history may be deleted or edited by user
--    there is no guarantee of any set_id in task history still exist in workset_lst
--    or contain same input parameter values as it was at the time of task run.
--
CREATE TABLE task_run_set
(
  task_run_id INT NOT NULL, -- master key
  run_id      INT NOT NULL, -- if > 0 then result run id
  set_id      INT NOT NULL, -- if > 0 then input working set id used to run the model
  task_id     INT NOT NULL, -- link to task list
  PRIMARY KEY (task_run_id, run_id),
  CONSTRAINT task_rs_un
             UNIQUE (task_run_id, set_id),
  CONSTRAINT task_rs_mk
             FOREIGN KEY (task_run_id) REFERENCES task_run_lst (task_run_id)
);
