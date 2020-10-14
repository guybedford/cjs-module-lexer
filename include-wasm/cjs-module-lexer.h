#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>

extern unsigned char __heap_base;

const uint16_t* source = (void*)&__heap_base;
uint32_t parse_error;

struct Slice {
  const uint16_t* start;
  const uint16_t* end;
  struct Slice* next;
};
typedef struct Slice Slice;

struct StarExportBinding {
  const uint16_t* specifier_start;
  const uint16_t* specifier_end;
  const uint16_t* id_start;
  const uint16_t* id_end;
};
typedef struct StarExportBinding StarExportBinding;

Slice* first_export = NULL;
Slice* export_read_head = NULL;
Slice* export_write_head = NULL;
Slice* first_reexport = NULL;
Slice* reexport_read_head = NULL;
Slice* reexport_write_head = NULL;
Slice* first_require = NULL;
Slice* require_read_head = NULL;
Slice* require_write_head = NULL;
void* analysis_base;
void* analysis_head;

void bail (uint32_t err);

// allocateSource
const uint16_t* sa (uint32_t utf16Len) {
  const uint16_t* sourceEnd = source + utf16Len + 1;
  // ensure source is null terminated
  *(uint16_t*)(source + utf16Len) = '\0';
  analysis_base = (void*)sourceEnd;
  analysis_head = analysis_base;
  first_export = NULL;
  export_write_head = NULL;
  export_read_head = NULL;
  first_reexport = NULL;
  reexport_write_head = NULL;
  reexport_read_head = NULL;
  first_require = NULL;
  require_write_head = NULL;
  require_read_head = NULL;
  return source;
}

// getErr
uint32_t e () {
  return parse_error;
}

// getExportStart
uint32_t es () {
  return export_read_head->start - source;
}
// getExportEnd
uint32_t ee () {
  return export_read_head->end - source;
}
// getReexportStart
uint32_t res () {
  return reexport_read_head->start - source;
}
// getReexportEnd
uint32_t ree () {
  return reexport_read_head->end - source;
}
// getRequireStart
uint32_t rqs () {
  return require_read_head->start - source;
}
// getRequireEnd
uint32_t rqe () {
  return require_read_head->end - source;
}
// readExport
bool re () {
  if (export_read_head == NULL)
    export_read_head = first_export;
  else
    export_read_head = export_read_head->next;
  if (export_read_head == NULL)
    return false;
  return true;
}
// readReexport
bool rre () {
  if (reexport_read_head == NULL)
    reexport_read_head = first_reexport;
  else
    reexport_read_head = reexport_read_head->next;
  if (reexport_read_head == NULL)
    return false;
  return true;
}
// readRequire
bool rrq () {
  if (require_read_head == NULL)
    require_read_head = first_require;
  else
    require_read_head = require_read_head->next;
  if (require_read_head == NULL)
    return false;
  return true;
}

bool parse (uint32_t point);

void _addExport (const uint16_t* start, const uint16_t* end) {
  Slice* export = (Slice*)(analysis_head);
  analysis_head = analysis_head + sizeof(Slice);
  if (export_write_head == NULL)
    first_export = export;
  else
    export_write_head->next = export;
  export_write_head = export;
  export->start = start;
  export->end = end;
  export->next = NULL;
}
void _addReexport (const uint16_t* start, const uint16_t* end) {
  Slice* reexport = (Slice*)(analysis_head);
  analysis_head = analysis_head + sizeof(Slice);
  if (reexport_write_head == NULL)
    first_reexport = reexport;
  else
    reexport_write_head->next = reexport;
  reexport_write_head = reexport;
  reexport->start = start;
  reexport->end = end;
  reexport->next = NULL;
}
void _addRequire (const uint16_t* start, const uint16_t* end) {
  Slice* require = (Slice*)(analysis_head);
  analysis_head = analysis_head + sizeof(Slice);
  if (require_write_head == NULL)
    first_require = require;
  else
    require_write_head->next = require;
  require_write_head = require;
  require->start = start;
  require->end = end;
  require->next = NULL;
}
void (*addExport)(const uint16_t*, const uint16_t*) = &_addExport;
void (*addReexport)(const uint16_t*, const uint16_t*) = &_addReexport;
void (*addRequire)(const uint16_t*, const uint16_t*) = &_addRequire;
bool parseCJS (
  uint16_t* source,
  uint32_t sourceLen,
  void (*addExport)(const uint16_t* start, const uint16_t* end),
  void (*addReexport)(const uint16_t* start, const uint16_t* end),
  void (*addRequire)(const uint16_t* start, const uint16_t* end)
);

enum RequireType {
  Import,
  ExportAssign,
  ExportStar
};

void tryBacktrackAddStarExportBinding (uint16_t* pos);
bool tryParseRequire (enum RequireType requireType);
void tryParseLiteralExports ();
bool readExportsOrModuleDotExports (uint16_t ch);
void tryParseModuleExportsDotAssign ();
void tryParseExportsDotAssign (bool assign);
void tryParseObjectDefineOrKeys (bool keys);
bool identifier (uint16_t ch);

void throwIfImportStatement ();
void throwIfExportStatement ();

void readImportString (const uint16_t* ss, uint16_t ch);
uint16_t readExportAs (uint16_t* startPos, uint16_t* endPos);

uint16_t commentWhitespace ();
void singleQuoteString ();
void doubleQuoteString ();
void regularExpression ();
void templateString ();
void blockComment ();
void lineComment ();

uint16_t readToWsOrPunctuator (uint16_t ch);

uint32_t fullCharCode (uint16_t ch);
uint32_t fullCharCodeAtLast (uint16_t* pos);
bool isIdentifierStart (uint32_t code);
bool isIdentifierChar (uint32_t code);
int charCodeByteLen (uint32_t ch);

bool isBr (uint16_t c);
bool isBrOrWs (uint16_t c);
bool isBrOrWsOrPunctuator (uint16_t c);
bool isBrOrWsOrPunctuatorNotDot (uint16_t c);

bool str_eq2 (uint16_t* pos, uint16_t c1, uint16_t c2);
bool str_eq3 (uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3);
bool str_eq4 (uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4);
bool str_eq5 (uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4, uint16_t c5);
bool str_eq6 (uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4, uint16_t c5, uint16_t c6);
bool str_eq7 (uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4, uint16_t c5, uint16_t c6, uint16_t c7);
bool str_eq9 (uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4, uint16_t c5, uint16_t c6, uint16_t c7, uint16_t c8, uint16_t c9);
bool str_eq10 (uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4, uint16_t c5, uint16_t c6, uint16_t c7, uint16_t c8, uint16_t c9, uint16_t c10);
bool str_eq13 (uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4, uint16_t c5, uint16_t c6, uint16_t c7, uint16_t c8, uint16_t c9, uint16_t c10, uint16_t c11, uint16_t c12, uint16_t c13);
bool str_eq18 (uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4, uint16_t c5, uint16_t c6, uint16_t c7, uint16_t c8, uint16_t c9, uint16_t c10, uint16_t c11, uint16_t c12, uint16_t c13, uint16_t c14, uint16_t c15, uint16_t c16, uint16_t c17, uint16_t c18);

bool readPrecedingKeyword2(uint16_t* pos, uint16_t c1, uint16_t c2);
bool readPrecedingKeyword3(uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3);
bool readPrecedingKeyword4(uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4);
bool readPrecedingKeyword5(uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4, uint16_t c5);
bool readPrecedingKeyword6(uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4, uint16_t c5, uint16_t c6);
bool readPrecedingKeyword7(uint16_t* pos, uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4, uint16_t c5, uint16_t c6, uint16_t c7);

bool keywordStart (uint16_t* pos);
bool isExpressionKeyword (uint16_t* pos);
bool isParenKeyword (uint16_t* pos);
bool isPunctuator (uint16_t charCode);
bool isExpressionPunctuator (uint16_t charCode);
bool isExpressionTerminator (uint16_t* pos);

void nextChar (uint16_t ch);
void nextCharSurrogate (uint16_t ch);
uint16_t readChar ();

void syntaxError ();
