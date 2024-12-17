CREATE FUNCTION block_unsupported_statements() RETURNS void
AS '$libdir/block_unsupported', 'block_unsupported_statements'
LANGUAGE c STRICT;