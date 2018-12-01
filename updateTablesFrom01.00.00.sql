create table "annualStatement" (
  "id" integer primary key autoincrement,
  "version" integer not null,
  "annualStatements_id" bigint,
  "referenceDate" text,
  "balance" integer not null,
  constraint "fk_annualStatement_annualStatements" foreign key ("annualStatements_id") references "user" ("id") deferrable initially deferred
);
