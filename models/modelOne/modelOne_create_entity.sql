--
-- Copyright (c) 2013-2022 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--

--
-- modelOne entities: Person and Other
-- digests value below is a test value and not a real digest, real digests are 32 digits hex
--

--
-- Person entity attributes:
--   age (int), ageGroup, sex, income (double), salary (double), salaryGroup, fullTime, isOldAge (bool), pension (double)
--
-- Other entity attributes:
--   age (int), ageGroup, income (double), isOldAge (bool)
--
-- additional attributes for large model version:
--   years (int), period, low, middle
--
INSERT INTO entity_dic      (entity_hid, entity_name, entity_digest) VALUES (16, 'Person', '_20120817160459d16');
INSERT INTO entity_dic      (entity_hid, entity_name, entity_digest) VALUES (17, 'Other', '_20120817160459e17');
INSERT INTO model_entity_dic (model_id, model_entity_id, entity_hid)  VALUES (1, 0, 16);
INSERT INTO model_entity_dic (model_id, model_entity_id, entity_hid)  VALUES (1, 1, 17);

INSERT INTO entity_dic_txt (entity_hid, lang_id, descr, note) VALUES (16, 0, 'Person entity',   'Person entity notes in English');
INSERT INTO entity_dic_txt (entity_hid, lang_id, descr, note) VALUES (16, 1, 'Entité personne', 'Notes d''entité personne en français');
INSERT INTO entity_dic_txt (entity_hid, lang_id, descr, note) VALUES (17, 0, 'Other entity',    'Other entity notes in English');
INSERT INTO entity_dic_txt (entity_hid, lang_id, descr, note) VALUES (17, 1, 'Autre entité',    'Autres notes d''entité en français');

INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (16, 0, 'Age',           4, 0);
INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (16, 1, 'AgeGroup',    101, 0);
INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (16, 2, 'Sex',         102, 0);
INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (16, 3, 'Income',       14, 0);
INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (16, 4, 'Salary',       14, 0);
INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (16, 5, 'SalaryGroup', 103, 0);
INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (16, 6, 'FullTime',    104, 0);
INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (16, 7, 'IsOldAge',      7, 0);
INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (16, 8, 'Pension',      14, 0);
INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (16, 9, 'crc',          12, 1);

INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (17, 20, 'Age',          4, 0);
INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (17, 21, 'AgeGroup',   101, 0);
INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (17, 22, 'Income',      14, 0);
INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (17, 23, 'crc',         12, 1);

INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 0, 0, 'Age of person', 'Person age notes in English');
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 0, 1, 'Âge de la personne', 'Notes d''âge de la personne en français');
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 1, 0, 'Age group', 'Age group may not correspond to age value');
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 1, 1, 'Tranche d''âge', 'Le groupe d''âge peut ne pas correspondre à la valeur d''âge');
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 2, 0, 'Gender', NULL);
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 2, 1, 'Le genre', NULL);
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 3, 0, 'Income', NULL);
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 3, 1, 'Revenu', NULL);
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 4, 0, 'Salary', NULL);
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 4, 1, 'Un salaire', NULL);
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 5, 0, 'Salary Group', 'Salary group may not correspond to salary value');
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 5, 1, 'Groupe salarial', 'Le groupe salarial peut ne pas correspondre à la valeur salariale');
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 6, 0, 'Full or part time', NULL);
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 6, 1, 'Temps plein ou partiel', NULL);
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 7, 0, 'Is Old Age', 'Old age security or guaranteed income');
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 7, 1, 'Est la vieillesse', 'Sécurité de la vieillesse ou revenu garanti');
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 8, 0, 'Pension', 'Pension or disability income');
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (16, 8, 1, 'Pension', 'Revenu de pension ou d''invalidité');

INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (17, 20, 0, 'Age of others',  'Other age notes in English');
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (17, 20, 1, 'Âge des autres', 'Autres notes d''âge en français');
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (17, 21, 0, 'Age group', 'Age group may not correspond to age value');
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (17, 21, 1, 'Tranche d''âge', 'Le groupe d''âge peut ne pas correspondre à la valeur d''âge');
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (17, 22, 0, 'Income', NULL);
INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (17, 22, 1, 'Revenu', NULL);
