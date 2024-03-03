--
-- Copyright (c) 2014 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--

-- BEGIN TRANSACTION;

--
-- list of ids, must be positive.
-- id < 100 reserved for development and testing
-- also for types id < 100 reserved for bult-in types
--
INSERT INTO id_lst (id_key, id_value) VALUES ('openmpp',       104);
INSERT INTO id_lst (id_key, id_value) VALUES ('lang_id',       100);
INSERT INTO id_lst (id_key, id_value) VALUES ('model_id',      100);
INSERT INTO id_lst (id_key, id_value) VALUES ('type_hid',      100);
INSERT INTO id_lst (id_key, id_value) VALUES ('parameter_hid', 100);
INSERT INTO id_lst (id_key, id_value) VALUES ('table_hid',     100);
INSERT INTO id_lst (id_key, id_value) VALUES ('run_id_set_id', 100);
INSERT INTO id_lst (id_key, id_value) VALUES ('entity_hid',    100);

--
-- Languages and word list
--
INSERT INTO lang_lst (lang_id, lang_code, lang_name) VALUES (0, 'EN', 'English');
INSERT INTO lang_lst (lang_id, lang_code, lang_name) VALUES (1, 'FR', 'Français');

-- built-in words
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'all', 'All');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'min', 'Min');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'max', 'Max');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'all', 'Toutes');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'min', 'Min');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'max', 'Max');

-- model runtime messages from library
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'Model', 'Model');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Model',
                                                                  'Modèle');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'FAILED', 'FAILED');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'FAILED',
                                                                  'PAS RÉUSSI');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'Done.', 'Done.');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Done.',
                                                                  'Fini.');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'Parallel run of %d modeling processes, %d thread(s) each', 'Parallel run of %d modeling processes, %d thread(s) each');

INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'Read', 'Read');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Read',
                                                                  'Lecture');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'Writing Output Tables Expressions', 'Writing Output Tables Expressions');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Writing Output Tables Expressions',
                                                                  'Enregistrement des expressions des tableaux de sorti');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'Run: %d', 'Run: %d');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Run: %d',
                                                                  'Exécution: %d');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'Sub-value %d', 'Sub-value %d');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Sub-value %d',
                                                                  'Sous échantillon %d');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'Restart from sub-value %d', 'Restart from sub-value %d');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Restart from sub-value %d',
                                                                  'Redemarrer du sous-échantillon %d');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'MPI error: send %d unfinished from %d to %d', 'MPI error: send %d unfinished from %d to %d');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'MPI error: send %d unfinished from %d to %d',
                                                                  'erreur MPI : envoie %d inachevé de %d à %d');

-- model runtime messages from use .mpp .ompp
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'Member=%d create starting population', 'Member=%d create starting population');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Member=%d create starting population',
                                                                  'Member=%d créer la population de départ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'Member=%d simulation progress=%d%%', 'Member=%d simulation progress=%d%%');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Member=%d simulation progress=%d%%',
                                                                  'Membre=%d progrès de la simulation=%d%%');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'error : Event time ', 'error : Event time ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'error : Event time ',
                                                                  'erreur : Le temps d''évènement ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, ' is earlier than current time ', ' is earlier than current time ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, ' is earlier than current time ',
                                                                  ' est plus tôt que le temps actuel ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, ' in simulation member ', ' in simulation member ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, ' in simulation member ',
                                                                  ' dans le membre de simulation ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, ' in event ', ' in event ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, ' in event ',
                                                                  ' dans l''évènement ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, ' in entity_id ', ' in entity_id ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, ' in entity_id ',
                                                                  ' dans entity_id ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, ' with combined seed ', ' with combined seed ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, ' with combined seed ',
                                                                  ' avec graine aléatoire combinée ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'error : stream number ', 'error : stream number ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'error : stream number ',
                                                                  'erreur : La fluxe aléatoire numéro ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, ' exceeds the maximum number of streams ', ' exceeds the maximum number of streams ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, ' exceeds the maximum number of streams ',
                                                                  ' dépasse le nombre maximal de fluxes ');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, ' Increase the number of streams in ompp_framework.ompp.', ' Increase the number of streams in ompp_framework.ompp.');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, ' Increase the number of streams in ompp_framework.ompp.',
                                                                  ' Augmentez le nombre de fluxes aléatoires dans ompp_framework.ompp.');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Error - SimulationSeed ''%lld'' must be greater than 0',
                                                                  'Erreur - SimulationSeed ''%lld'' doit être supérieur à 0');

-- model runtime messages from generated code in om_definitions.cpp (by code order in CodeGen.cpp)
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Model build    : % s % s % s',
                                                                  'Construction du modèle : % s % s % s');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Warning : parameter ''%s'' was missing when the model was built',
                                                                  'Attention : le paramètre ''%s'' était manquant lors de la construction du modèle');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Prepare fixed and missing parameters',
                                                                  'Préparer les paramètres fixes et manquants');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Note : this Debug model was built with Microsoft iterator debug disabled',
                                                                  'Note : ce modèle de débogage a été compilé avec le débogage de l’itérateur Microsoft désactivé');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Warning : prohibited attribute assignment not detected with verify_attribute_modification = off',
                                                                  'Attention : l''affectation d''attribut interdite n''est pas détectée avec verify_attribute_modification = off');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Warning : prohibited time-like attribute access is not detected with verify_timelike_attribute_access = off',
                                                                  'Attention : l''accès aux attributs temporels interdits n''est pas détecté avec verify_timelike_attribute_access = off');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Warning : invalid table increment is not detected with verify_valid_table_increment = off',
                                                                  'Attention : l''incrément de tableau invalide n''est pas détecté avec verify_valid_table_increment = off');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Warning : model can expose microdata at run-time with output_microdata = on',
                                                                  'Attention : le modèle peut exposer des microdonnées au moment de l''exécution quand output_microdata = on');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Note : model is weight-enabled and microdata-enabled, include entity_weight in Microdata for downstream weighted operations',
                                                                  'Note : le modèle est construit avec le poids et les microdonnées, incluez entity_weight dans Microdonnées pour les opérations pondérées en aval');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Warning : model can expose microdata at run-time with event_trace = on',
                                                                  'Attention : le modèle peut exposer des microdonnées au moment de l''exécution avec event_trace = on');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'Get scenario parameters for process',
                                                                  'Obtenir les paramètres de scénario pour le processus');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'member=%d Bind scenario parameters',
                                                                  'membre=%d Lier les paramètres du scénario');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'member=%d Predicted memory required = %d MB per parallel sub and %d MB per instance',
                                                                  'membre=%d Mémoire prédite requise = %d Mo par membre parallèle et %d Mo par instance');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'member=%d Compute derived parameters',
                                                                  'membre=%d Calculer les paramètres dérivés');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'member=%d Prepare for simulation',
                                                                  'membre=%d Préparer la simulation');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'member=%d Compute entity tables',
                                                                  'membre=%d Calculer les tableaux d''entité');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'member=%d Compute post-simulation',
                                                                  'membre=%d Calcul post-simulation');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'member=%d Compute derived tables',
                                                                  'membre=%d Calculer les tableaux dérivés');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'member=%d Write output tables - start',
                                                                  'membre=%d Ecrire les tableaux de sortie - début');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'member=%d Write output tableaux - finish',
                                                                  'membre=%d Ecrire les tables de sortie - fin');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'member=%d Write derived parameters - start',
                                                                  'membre=%d Ecrire les paramètres dérivés - début');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'member=%d Write derived parameters - finish',
                                                                  'membre=%d Ecrire les paramètres dérivés - fin');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'member=%d Process peak memory usage: %.2f MB',
                                                                  'membre=%d Utilisation maximale de la mémoire du processus : %.2f Mo');


																  
--
-- built-in types: type name used as unique type digest
--
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (1, 'char',  '_char_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (2, 'schar', '_schar_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (3, 'short', '_short_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (4, 'int',   '_int_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (5, 'long',  '_long_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (6, 'llong', '_llong_', 0, 1);
-- INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (7, 'bool',  '_bool_', 1, 2);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (8, 'uchar',    '_uchar_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (9, 'ushort',   '_ushort_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (10, 'uint',     '_uint_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (11, 'ulong',   '_ulong_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (12, 'ullong',  '_ullong_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (13, 'float',   '_float_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (14, 'double',  '_double_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (15, 'ldouble', '_ldouble_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (16, 'Time',    '_Time_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (17, 'real',    '_real_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (18, 'integer', '_integer_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (19, 'counter', '_counter_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (20, 'big_counter', '_big_counter_', 0, 1);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (21, 'file',    '_file_', 0, 1);

-- 
-- built-in types: logical type
--
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (7, 'bool', '_bool_', 1, 2);

INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (7, 0, 'logical type', NULL);
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (7, 1, 'type logique', NULL);

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (7, 0, 'false');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (7, 1, 'true');

INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (7, 0, 0, 'False', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (7, 0, 1, 'Faux', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (7, 1, 0, 'True', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (7, 1, 1, 'Vrai', NULL);

-- COMMIT;
