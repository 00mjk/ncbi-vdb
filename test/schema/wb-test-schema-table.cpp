/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

/**
* Unit tests for table declarations in schema, this file is #included into a bigger test suite
*/

class TableAccess // encapsulates access to an STable in a VSchema
{
public:
    TableAccess ( const STable* p_fn )
    : m_self ( p_fn )
    {
    }

    /* optional function for determining table type */
    const SFunction * Untyped () const { return m_self -> untyped; }

    /* optional expression for limiting all columns' blob sizes */
    const SExpression * Limit () const { return m_self -> limit; }

    /* optional default view declaration */
    const String * DefaultView () const  { return m_self -> dflt_view; }

    /* scope */
    const BSTree & Scope () const { return m_self -> scope; }

    /* parents */
    const Vector & Parents () const { return m_self -> parents; }

    /* overrides ( inherited virtual productions )
       contents are grouped by introducing parent */
    const Vector & Overrides () const { return m_self -> overrides; }
    uint32_t OverrideCount () const { return VectorLength ( & m_self -> overrides ); }
    uint32_t OverrideStart () const { return VectorStart ( & m_self -> overrides ); }

    /* columns */
    uint32_t ColumnCount () const { return VectorLength ( & m_self -> col ); }
    const SColumn * GetColumn ( uint32_t p_idx ) const
    {
        return static_cast < const SColumn * > ( VectorGet ( & m_self -> col, p_idx ) );
    }
    uint32_t ColumnNameCount () const { return VectorLength ( & m_self -> cname ); }
    const SNameOverload * GetColumnName ( uint32_t p_idx ) const
    {
        return static_cast < const SNameOverload * > ( VectorGet ( & m_self -> cname, p_idx ) );
    }

    /* explicitly declared physical column members */
    uint32_t PhysicalColumnCount () const { return VectorLength ( & m_self -> phys ); }
    const SPhysMember * GetPhysicalColumn ( uint32_t p_idx ) const
    {
        return static_cast < const SPhysMember * > ( VectorGet ( & m_self -> phys, p_idx ) );
    }

    /* assignment statements */
    const Vector & Productions () const { return m_self -> prod; }
    uint32_t ProductionCount () const { return VectorLength ( & m_self -> prod ); }
    uint32_t ProductionStart () const { return VectorStart ( & m_self -> prod ); }
    const SProduction * GetProduction ( uint32_t p_idx ) const
    {
        return static_cast < const SProduction * > ( VectorGet ( & m_self -> prod, ProductionStart () + p_idx ) );
    }

    /* introduced virtual ( undefined ) productions
       contents are unowned KSymbol pointers */
    uint32_t VirtualProductionCount () const { return VectorLength ( & m_self -> vprods ); }
    uint32_t VirtualProductionStart () const { return VectorStart ( & m_self -> vprods ); }
    const KSymbol * GetVirtualProduction ( uint32_t p_idx ) const
    {
        return static_cast < const KSymbol * > ( VectorGet ( & m_self -> vprods, VirtualProductionStart () + p_idx ) );
    }

    /* owned KSymbols that are not in scope */
    uint32_t OwnedSymbolCount () const { return VectorLength ( & m_self -> syms ); }
    uint32_t OwnedSymbolStart () const { return VectorStart ( & m_self -> syms ); }
    const KSymbol * GetOwnedSymbol ( uint32_t p_idx ) const
    {
        return static_cast < const KSymbol * > ( VectorGet ( & m_self -> syms, OwnedSymbolStart () + p_idx ) );
    }

    /* source file & line */
    string SourceFile () const { return AST_Fixture :: ToCppString ( m_self -> src_file ); }
    uint32_t SpourceLine () const { return m_self -> src_line; }

    /* required version */
    uint32_t Version () const { return m_self -> version; }

    /* table id */
    uint32_t Id () const { return m_self -> id; }

    /* marking */
    bool IsMarked () const { return m_self -> marked; }

    uint32_t OverloadCount () const
    {
        const SNameOverload * ovl = static_cast < const SNameOverload * > ( m_self -> name -> u . obj );
        return VectorLength ( & ovl -> items );
    }
    const STable * GetOverload ( uint32_t p_idx ) const
    {
        const SNameOverload * ovl = static_cast < const SNameOverload * > ( m_self -> name -> u . obj );
        return static_cast < const STable * > ( VectorGet ( & ovl -> items, p_idx ) );
    }

    const STable *m_self;
};

class AST_Table_Fixture : public AST_Fixture
{
public:
    AST_Table_Fixture ()
    {
    }
    ~AST_Table_Fixture ()
    {
    }

    TableAccess ParseTable ( const char * p_source, const char * p_name, uint32_t p_idx = 0 )
    {
        MakeAst ( p_source );

        const STable * ret = static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, p_idx ) );
        if ( ret == 0 || ret -> name == 0 || string ( p_name ) != ToCppString ( ret -> name -> name ) )
        {
            throw std :: logic_error ( "AST_Table_Fixture::ParseTable : wrong name" );
        }
        return TableAccess ( ret );
    }

    uint32_t Version ( uint32_t p_major, uint32_t p_minor = 0, uint32_t p_release = 0 )
    {
        return ( p_major << 24 ) + ( p_minor << 16 ) + p_release;
    }

#define THROW_ON_TRUE(cond) if ( cond ) throw logic_error ( "VerifyImplicitPhysicalColumn: " #cond );
    void VerifyImplicitPhysicalColumn ( const SColumn & p_col )
    {
        // a physical column is attached to p_col
        THROW_ON_TRUE ( p_col . read  == 0 );
        const SSymExpr & read = * reinterpret_cast < const SSymExpr * > ( p_col . read );
        THROW_ON_TRUE ( ePhysExpr != read . dad . var );
        THROW_ON_TRUE ( read . _sym == 0 );
        THROW_ON_TRUE ( string ( ".c" ) != ToCppString ( read . _sym -> name ) );
        THROW_ON_TRUE ( ePhysMember != read . _sym -> type );
        // ...with a read rule...
        THROW_ON_TRUE ( read . _sym -> u . obj == 0 );
        const SPhysMember & phys = * reinterpret_cast < const SPhysMember * > ( read . _sym -> u . obj );
        THROW_ON_TRUE ( ! phys . simple );
        THROW_ON_TRUE ( p_col . td . type_id != phys . td . type_id );
        THROW_ON_TRUE ( p_col . td . dim     != phys . td . dim );
        THROW_ON_TRUE ( phys . type != 0 );
        THROW_ON_TRUE ( read . _sym != phys . name );
        // ... that reads the value of p_col
        THROW_ON_TRUE ( phys . expr == 0 );
        const SSymExpr & colExpr = * reinterpret_cast < const SSymExpr * > ( phys . expr );
        THROW_ON_TRUE ( eColExpr != colExpr . dad . var );
        THROW_ON_TRUE ( p_col . name != colExpr . _sym );
    }
#undef THROW_ON_TRUE
};

FIXTURE_TEST_CASE(Table_Empty, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1.1 { };", "t" );
    REQUIRE_EQ ( ( 1u << 24 ) + ( 1u << 16 ), t . Version () );
    REQUIRE_EQ ( 0u, t . Id () );

    // verify entry in the container of (overloaded) table names
    const SNameOverload * ovl = static_cast < const SNameOverload * > ( t . m_self -> name -> u . obj );
    REQUIRE_EQ ( ( const void * ) ovl, ( const void * ) VectorGet ( & GetSchema () -> tname, 0 ) );

    REQUIRE_EQ ( 1u, t . OverloadCount () );
    REQUIRE_EQ ( t . m_self, t. GetOverload (  0 ) );
}

// version

FIXTURE_TEST_CASE(Table_Exists_SameVersion, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1.2.3 { }; table t#1.2.3 { };", "t" );
    // one declaration ignored
    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> tbl ) );
    REQUIRE_EQ ( 1u, t . OverloadCount () );
    REQUIRE_EQ ( Version ( 1, 2, 3 ), t. GetOverload (  0 ) -> version );
}
FIXTURE_TEST_CASE(Table_Exists_OlderMinorVersion, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1.2 { }; table t#1.1 { };", "t" );
    // older version ignored
    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> tbl ) );
    REQUIRE_EQ ( 1u, t . OverloadCount () );
    REQUIRE_EQ ( Version ( 1, 2 ), t. GetOverload (  0 ) -> version );
}
FIXTURE_TEST_CASE(Table_Exists_NewerMinorVersion, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1.1 { }; table t#1.2 { };", "t" );
    // both versions recorded, only newer one used in the overloads table
    REQUIRE_EQ ( 2u, VectorLength ( & GetSchema () -> tbl ) );
    REQUIRE_EQ ( 1u, t . OverloadCount () );
    REQUIRE_EQ ( Version ( 1, 2 ), t. GetOverload (  0 ) -> version );
}
FIXTURE_TEST_CASE(Table_Exists_OlderMajorVersion, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#2 { }; table t#1 { };", "t" );
    // both versions recorded, both used in the overloads table
    REQUIRE_EQ ( 2u, VectorLength ( & GetSchema () -> tbl ) );
    REQUIRE_EQ ( 2u, t . OverloadCount () );
    REQUIRE_EQ ( Version ( 1 ), t. GetOverload (  0 ) -> version );
    REQUIRE_EQ ( Version ( 2 ), t. GetOverload (  1 ) -> version );
}
FIXTURE_TEST_CASE(Table_Exists_NewerMajorVersion, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { }; table t#2 { };", "t" );
    // both versions recorded, both used in the overloads table
    REQUIRE_EQ ( 2u, VectorLength ( & GetSchema () -> tbl ) );
    REQUIRE_EQ ( 2u, t . OverloadCount () );
    REQUIRE_EQ ( Version ( 1 ), t. GetOverload (  0 ) -> version );
    REQUIRE_EQ ( Version ( 2 ), t. GetOverload (  1 ) -> version );
}
FIXTURE_TEST_CASE(Table_Exists_OlderRelease, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1.2.3 { }; table t#1.2.1 { };", "t" );
    // older version ignored
    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> tbl ) );
    REQUIRE_EQ ( 1u, t . OverloadCount () );
    REQUIRE_EQ ( Version ( 1, 2, 3 ), t. GetOverload (  0 ) -> version );
}
FIXTURE_TEST_CASE(Table_Exists_NewerRelease, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1.2.3 { }; table t#1.2.4 { };", "t" );
    // both versions recorded, both used in the overloads table
    REQUIRE_EQ ( 2u, VectorLength ( & GetSchema () -> tbl ) );
    REQUIRE_EQ ( 1u, t . OverloadCount () );
    REQUIRE_EQ ( Version ( 1, 2, 4 ), t. GetOverload (  0 ) -> version ); // newer
}

// parents

FIXTURE_TEST_CASE(Table_OneParent, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table p#1 { }; table t#1 = p { };", "t", 1 );
    REQUIRE_EQ ( 1u, VectorLength ( & t . Parents () ) );
    REQUIRE_EQ ( VectorGet ( & GetSchema () -> tbl, 0 ), VectorGet ( & t . Parents (), 0 ) );
}

FIXTURE_TEST_CASE(Table_ParentDifferentMajorVersion, AST_Table_Fixture)
{
    VerifyErrorMessage  ( "table p#2 { }; table t#1 = p#1 { };", "Requested version does not exist: 'p#1'" );
}
FIXTURE_TEST_CASE(Table_ParentOlderMinorVersion, AST_Table_Fixture)
{   // requesting an older minor version is not a problem
    TableAccess t = ParseTable ( "table p#1.2 { }; table t#1 = p#1.1 { };", "t", 1 );
    REQUIRE_EQ ( 1u, VectorLength ( & t . Parents () ) );
}
FIXTURE_TEST_CASE(Table_ParentNewerMinorVersion, AST_Table_Fixture)
{
    VerifyErrorMessage  ( "table p#1.1 { }; table t#1 = p#1.2 { };", "Requested version does not exist: 'p#1.2'" );
}
FIXTURE_TEST_CASE(Table_ParentNewerRelease, AST_Table_Fixture)
{
    VerifyErrorMessage  ( "table p#1.1.1 { }; table t#1 = p#1.1.2 { };", "Requested version does not exist: 'p#1.1.2'" );
}
FIXTURE_TEST_CASE(Table_ParentOlderRelease, AST_Table_Fixture)
{   // requesting an older release is not a problem
    TableAccess t = ParseTable ( "table p#1.1.2 { }; table t#1 = p#1.1.1 { };", "t", 1 );
    REQUIRE_EQ ( 1u, VectorLength ( & t . Parents () ) );
    TableAccess p ( static_cast < const STable * > ( VectorGet ( & t . Parents (), 0 ) ) );
    REQUIRE_EQ ( 1u, p . OverloadCount () );
    REQUIRE_EQ ( Version ( 1, 1, 2 ), p. GetOverload (  0 ) -> version );
}

FIXTURE_TEST_CASE(Table_ParentNewestVersionSelected, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table p#1.1 { }; table p#1.2 { }; table p#1.3 { }; table t#1 = p#1.2 { };", "t", 3 );
    REQUIRE_EQ ( 1u, VectorLength ( & t . Parents () ) );
    TableAccess p ( static_cast < const STable * > ( VectorGet ( & t . Parents (), 0 ) ) );
    REQUIRE_EQ ( 1u, p . OverloadCount () );
    REQUIRE_EQ ( Version ( 1, 3 ), p. GetOverload (  0 ) -> version );
}


FIXTURE_TEST_CASE(Table_MultipleParents, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table p1#1 { }; table p2#1 { }; table t#1 = p1, p2 { };", "t", 2 );
    REQUIRE_EQ ( 2u, VectorLength ( & t . Parents () ) );
}

FIXTURE_TEST_CASE(Table_ParentRedeclared, AST_Table_Fixture)
{   // parent redeclared with a higher minor version: children updated
    TableAccess t = ParseTable ( "table p1#1.1 { }; table t#1 = p1 { }; table p1#1.2 { }; ", "t", 1 );
    REQUIRE_EQ ( 1u, VectorLength ( & t . Parents () ) );
    TableAccess p ( static_cast < const STable * > ( VectorGet ( & t . Parents (), 0 ) ) );
    REQUIRE_EQ ( Version ( 1, 2 ), p. Version () );
}

FIXTURE_TEST_CASE(Table_Parent_Override, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table dad#1 { U8 a = c; } table t#1 = dad#1 { U16 b = 1; }", "t", 1 );
    // c is inherited and is added to Overrides
    REQUIRE_EQ ( 1u, t . ProductionCount () );
    REQUIRE_EQ ( 1u, t . OverrideCount () );
}

FIXTURE_TEST_CASE(Table_Parent_InheritedVirtualProduction, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table dad#1 { U8 a = c; } table t#1 = dad#1 { U16 b = c; }", "t", 1 );
    // c is not seen as introduced in child table
    REQUIRE_EQ ( 0u, t . VirtualProductionCount () );
}

FIXTURE_TEST_CASE(Table_Parent_MultuipleInheritedOverride, AST_Table_Fixture)
{
    TableAccess t = ParseTable (
        "table dad1#1 { U8 a1 = c; }\n"
        "table dad2#1 { U8 a2 = c; }\n"
        "table t#1 = dad1#1, dad2#1 { U16 b = c; }",
        "t",
        2 );
    // c is inherited twice and is added to Overrides twice
    REQUIRE_EQ ( 2u, t . OverrideCount () );
    REQUIRE_EQ ( 0u, t . VirtualProductionCount () ); // c is not seen as introduced in child table
}

FIXTURE_TEST_CASE(Table_Parent_NotAParent, AST_Table_Fixture)
{
    VerifyErrorMessage (
        "function U16 dad1#1(); table t#1 = dad1#1 { U16 b = c; }",
        "Not a table: 'dad1'" );
}

//TODO: introduce in one parent, resolve in another
//TODO: introduce in parent, resolve in child, make sure forward reference in parent is fixed

// table body

FIXTURE_TEST_CASE(Table_Production, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { U8 i = 1; }", "t" );
    REQUIRE_EQ ( 1u, VectorLength ( & t . Productions () ) );
    REQUIRE ( ! t . GetProduction ( 0 ) -> trigger );
}

FIXTURE_TEST_CASE(Table_Production_ForwardReference, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { extern default column U8 c = prod; U8 prod = 1; }", "t" );
    REQUIRE_EQ ( 1u, VectorLength ( & t . Productions () ) );
    REQUIRE ( ! t . GetProduction ( 0 ) -> trigger );
}

FIXTURE_TEST_CASE(Table_Production_ForwardReference_Bad, AST_Table_Fixture)
{
    VerifyErrorMessage ( "table t#1 { extern default column U8 c; U8 c = 1; }", "Production name is already in use: 'c'" );
}

FIXTURE_TEST_CASE(Table_Production_ForwardReference_Defined, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { U16 a = b; U16 b = 1; }", "t" );
    REQUIRE_EQ ( 2u, t . ProductionCount () );
    // once defined, b is not virtual anymore
    REQUIRE_EQ ( 0u, t . VirtualProductionCount () );
}

FIXTURE_TEST_CASE(Table_Trigger, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { trigger tr = 1; }", "t" );
    REQUIRE_EQ ( 1u, VectorLength ( & t . Productions () ) );
    REQUIRE ( t . GetProduction ( 0 ) -> trigger );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_Simple, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { column U8 c; }", "t" );
    REQUIRE_EQ ( 1u, t . ColumnCount () );
    REQUIRE_EQ ( 1u, t . ColumnNameCount () );

    // verify the contents
    const SColumn & c = * t . GetColumn ( 0 );
    REQUIRE_NOT_NULL ( c . name );
    REQUIRE_EQ ( ( uint32_t ) eColumn, c . name -> type );
    REQUIRE_EQ ( ( const void * ) t . GetColumnName ( 0 ), c . name -> u . obj );
    REQUIRE_NULL ( c . validate );
    REQUIRE_NULL ( c . limit );
    REQUIRE_NULL ( c . ptype );
    REQUIRE_EQ ( U8_id, c . td . type_id );
    REQUIRE_EQ ( 1u, c . td . dim);
    REQUIRE_EQ ( 0u, c . cid . ctx );
    REQUIRE_EQ ( 0u, c . cid . id );
    REQUIRE ( ! c . dflt );
    REQUIRE ( ! c . read_only );
    REQUIRE ( c . simple );

    // since there is no explict ".c", an implicit physical column ".c" is generated
    VerifyImplicitPhysicalColumn ( c );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_SimpleColumn_Typeset, AST_Table_Fixture)
{
    VerifyErrorMessage ( "typeset TypeSet {U8}; table t#1 { column TypeSet c; }",
                         "Cannot be used as a column type: 'TypeSet'" );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_PhysicalForwarded, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { U8 i = .c; column U8 c; }", "t" );
    REQUIRE_EQ ( 1u, t . ColumnCount () );
    VerifyImplicitPhysicalColumn ( * t . GetColumn ( 0 ) );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_Virtual, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { U8 i = .c; }", "t" );
    REQUIRE_EQ ( 0u, t . ColumnCount () );
    REQUIRE_EQ ( 1u, t . VirtualProductionCount () );
    const KSymbol * prod = t . GetVirtualProduction ( 0 );
    REQUIRE_EQ ( (uint32_t)eVirtual, prod -> type );
    REQUIRE_EQ ( 1u, t . OwnedSymbolCount () );
    REQUIRE_EQ ( prod, t . GetOwnedSymbol ( 0 ) );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_PhysicalVirtual, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table p#1 { U8 i = .c; } table t#1 = p#1 { column U8 c; }", "t", 1 );
    REQUIRE_EQ ( 1u, t . ColumnCount () );
    VerifyImplicitPhysicalColumn ( * t . GetColumn ( 0 ) );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_ModDefault, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { default column U8 c; }", "t" );
    const SColumn & c = * t . GetColumn ( 0 );
    REQUIRE ( c . dflt );
}
FIXTURE_TEST_CASE(Table_ColumnDecl_ModExtern, AST_Table_Fixture)
{
    ParseTable ( "table t#1 { extern column U8 c; }", "t" );  // allowed but gnored
}
FIXTURE_TEST_CASE(Table_ColumnDecl_ModReadOnly, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { readonly column U8 c { read = 1; } }", "t" );
    const SColumn & c = * t . GetColumn ( 0 );
    REQUIRE ( c . read_only );
}
FIXTURE_TEST_CASE(Table_ColumnDecl_ModAll, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { default extern readonly column U8 c { read = 1; } }", "t" );
    const SColumn & c = * t . GetColumn ( 0 );
    REQUIRE ( c . dflt );
    REQUIRE ( c . read_only );
}
FIXTURE_TEST_CASE(Table_ColumnDecl_ReadOnlyNoBody, AST_Table_Fixture)
{
    VerifyErrorMessage ( "table t#1 { readonly column U8 c; }", "Simple column cannot be readonly: 'c'", 1, 32 );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_ColumnExists, AST_Table_Fixture)
{
    VerifyErrorMessage ( "table t#1 { column U8 c; column U8 c; }", "Column already defined: 'c'", 1, 36 );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_PhysicalColumnOverload, AST_Table_Fixture)
{
    MakeAst ( "table t#1 { column U8 c { read = 1; }; column U16 c { read = 2; }; column U32 c { read = 3; } }" );
    //TODO: verify
}

FIXTURE_TEST_CASE(Table_ColumnDecl_PhysicalColumn_PredefineAndOverload, AST_Table_Fixture)
{
    MakeAst ( "table t#1 { column U8 c0 { read = c1; }; column U16 c1 { read = 2; }; column U32 c1 { read = 3; } }" );
    //TODO: verify
}

FIXTURE_TEST_CASE(Table_ColumnDecl_NotAType, AST_Table_Fixture)
{
    VerifyErrorMessage ( "table t1#1 {}; table t2#1 { column t1 c; }", "Cannot be used as a column type: 't1'" );
}

static
const SPhysEncExpr &
GetEncode ( const SColumn & p_col )
{
    assert ( p_col . ptype != 0 );
    return * reinterpret_cast < const SPhysEncExpr * > ( p_col . ptype );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_ReferenceToPhysical, AST_Table_Fixture)
{
    TableAccess t = ParseTable (
        "physical < type T > T ph #1 = { return 1; }"
        "table t#1 { column <U8> ph#1 c; }",
        "t", 0 );
    const SColumn & c = * t . GetColumn ( 0 );
    REQUIRE_NOT_NULL ( c . read );
    REQUIRE_EQ ( ( uint32_t ) ePhysExpr, c . read -> var );
    REQUIRE_NOT_NULL ( c . ptype );
    REQUIRE_EQ ( ( uint32_t ) ePhysEncExpr, c . ptype -> var );

    const SPhysEncExpr & enc = GetEncode ( c );

    REQUIRE_EQ ( 1u, VectorLength ( & enc . schem ) );
    REQUIRE_EQ ( U8_id , ( ( const STypeExpr * ) VectorGet ( & enc . schem, 0 ) ) -> dt -> id );

    REQUIRE ( enc . version_requested );
    REQUIRE_EQ ( Version ( 1 ), enc . version );

    REQUIRE_NOT_NULL ( enc . phys );
    REQUIRE_EQ ( string ( "ph" ), ToCppString ( enc . phys -> name -> name ) );

    REQUIRE_EQ ( 0u, VectorLength ( & enc . pfact ) );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_ReferenceToPhysical_BadVersion, AST_Table_Fixture)
{
    VerifyErrorMessage (
        "physical < type T > T ph #1.2 = { return 1; }"
        "table t#1 { column <U8> ph#100.200.300 c; }",
        "Requested version does not exist: 'ph#100.200.300'" );
}
FIXTURE_TEST_CASE(Table_ColumnDecl_PhysicalWithFactoryArgs, AST_Table_Fixture)
{
    TableAccess t = ParseTable (
        "physical < type T > T ph #1 < U8 i > = { return i; }"
        "table t#1 { column < U8 > ph#1 < 1 > c; }",
        "t", 0 );
    const SPhysEncExpr & enc = GetEncode ( * t . GetColumn ( 0 ) );
    REQUIRE_EQ ( 1u, VectorLength ( & enc . pfact ) );
    REQUIRE_EQ ( (uint32_t)eConstExpr , ( ( const SExpression * ) VectorGet ( & enc . pfact, 0 ) ) -> var );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_PhysicalNoSchemaArgsWithFactoryArgs, AST_Table_Fixture)
{
    TableAccess t = ParseTable (
        "physical U8 ph #1 < U8 i > = { return i; }"
        "table t#1 { column ph#1 < 1 > c; }",
        "t", 0 );
    const SPhysEncExpr & enc = GetEncode ( * t . GetColumn ( 0 ) );
    REQUIRE_EQ ( 1u, VectorLength ( & enc . pfact ) );
    REQUIRE_EQ ( (uint32_t)eConstExpr , ( ( const SExpression * ) VectorGet ( & enc . pfact, 0 ) ) -> var );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_PhysicalNoSchemaArgsNoFactoryArgs, AST_Table_Fixture)
{
    TableAccess t = ParseTable (
        "physical U8 ph #1 = { return 1; }"
        "table t#1 { column ph#1 c; }",
        "t", 0 );
    const SPhysEncExpr & enc = GetEncode ( * t . GetColumn ( 0 ) );
    REQUIRE_EQ ( 0u, VectorLength ( & enc . pfact ) );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_PhysicalNoSchemaArgsNoFactoryArgsNoVersion, AST_Table_Fixture)
{
    TableAccess t = ParseTable (
        "physical U8 ph #1 = { return 1; }"
        "table t#1 { column ph c; }",
        "t", 0 );
    const SPhysEncExpr & enc = GetEncode ( * t . GetColumn ( 0 ) );
    REQUIRE_EQ ( 0u, VectorLength ( & enc . pfact ) );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_ReferenceToPhysical_NotPhysical, AST_Table_Fixture)
{
    VerifyErrorMessage ( "table ph#1 {} table t#1 { column ph#1 c; }", "Not a physical encoding: 'ph'" );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_Physical_FormatAsSchemaArg, AST_Table_Fixture)
{
    TableAccess t = ParseTable (
        "physical < type T > T ph #1 = { return 1; }"
        "fmtdef fmt; "
        "table t#1 { column < fmt > ph#1  c; }",
        "t", 0 );
    const SPhysEncExpr & enc = GetEncode ( * t . GetColumn ( 0 ) );
    REQUIRE_EQ ( 1u, VectorLength ( & enc . schem ) );
    const STypeExpr & arg = * reinterpret_cast < const STypeExpr * > ( VectorGet ( & enc . schem, 0 ) );
    REQUIRE_NOT_NULL ( arg . fmt );
    REQUIRE_EQ ( string ( "fmt" ), ToCppString ( arg . fmt -> name -> name ) );
    REQUIRE_EQ ( arg . fd . fmt, arg . fmt -> id ) ;
}

FIXTURE_TEST_CASE(Table_ColumnDecl_Physical_TypesetAsSchemaArg, AST_Table_Fixture)
{
    VerifyErrorMessage (
        "physical < type T > T ph #1 = { return 1; }\n"
        "typeset ts { U8, U32 }; \n"
        "table t#1 { column < ts > ph#1 c; }",
        "Simple columns cannot have typeset as type: 'c'",
        3, 32 );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_SymConstAsSchemaArg, AST_Table_Fixture)
{
    TableAccess t = ParseTable (
        "physical < U8 C > U8 ph #1 = { return C; }"
        "const U8 c1 = 1; "
        "table t#1 { column < c1 > ph#1  c; }",
        "t" );
    const SPhysEncExpr & enc = GetEncode ( * t . GetColumn ( 0 ) );
    REQUIRE_EQ ( 1u, VectorLength ( & enc . schem ) );
    REQUIRE_EQ ( (uint32_t)eConstExpr,
                 reinterpret_cast < const SExpression * > ( VectorGet ( & enc . schem, 0 ) ) -> var );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_NonUintSymConstAsSchemaArg, AST_Table_Fixture)
{   // schema constant arguments must be uint
    if ( m_newParse ) // old parser does not check this condition, only mentions it in a comment
    {
        VerifyErrorMessage (
            "physical < U8 C > U8 ph #1 = { return C; }\n"
            "const I8 c1 = 1; \n"
            "table t#1 { column < c1 > ph#1  c; }",
            "Schema argument constant has to be an unsigned integer scalar: 'c1'",
            3, 22 );
    }
}

FIXTURE_TEST_CASE(Table_ColumnDecl_BadSchemaArg, AST_Table_Fixture)
{
    VerifyErrorMessage (
        "physical < U8 C > U8 ph #1 = { return C; }"
        "table t1#1 {} "
        "table t2#1 { column < t1 > ph#1  c; }",
        "Cannot be used as a schema parameter: 't1'" );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_UndefinedSchemaArg, AST_Table_Fixture)
{
    VerifyErrorMessage (
        "physical < U8 C > U8 ph #1 = { return C; }"
        "table t#1 { column < Undef > ph#1  c; }",
        "Undeclared identifier: 'Undef'" );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_Limit, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { column limit = 1; }", "t" );
    REQUIRE_NOT_NULL ( t . Limit () );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_LimitTwice, AST_Table_Fixture)
{
    VerifyErrorMessage ( "table t#1 { column limit = 1; column limit = 2; }", "Limit constraint already specified", 1, 46 );
}

#if WHEN_IMPLEMENTED_FUNCTION_CALL
TODO
FIXTURE_TEST_CASE(Table_ColumnDecl_LimitNonConst, AST_Table_Fixture)
{
    ParseTable (
        "extern function U8 f();"
        "table t#1 { column limit = f(); }",
        "Expression is not constant" );
}
#endif

FIXTURE_TEST_CASE(Table_ColumnDecl_DefaultLimit, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { column default limit = 1; }", "t" );
    REQUIRE_NOT_NULL ( t . Limit () );
}

FIXTURE_TEST_CASE(Table_ColumnDeclBody_Empty, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { column U8 c {} }", "t" );
    const SColumn & c = * t . GetColumn ( 0 );
    REQUIRE_NOT_NULL ( c . read ); // an implicit physical read is generated
    REQUIRE_NULL ( c . validate );
    REQUIRE_NULL ( c . limit );
    REQUIRE ( c . simple );
}

FIXTURE_TEST_CASE(Table_ColumnDeclBody_EmptyStmt, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { column U8 c { ; } }", "t" );
    const SColumn & c = * t . GetColumn ( 0 );
    REQUIRE_NOT_NULL ( c . read ); // an implicit physical read is generated
    REQUIRE_NULL ( c . validate );
    REQUIRE_NULL ( c . limit );
    REQUIRE ( c . simple );
}

FIXTURE_TEST_CASE(Table_ColumnDeclBody_Read, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { column U8 c { read = 1 | 2; } }", "t" );
    const SColumn & c = * t . GetColumn ( 0 );
    REQUIRE_NOT_NULL ( c . read );
    REQUIRE_EQ ( ( uint32_t ) eCondExpr, c . read -> var );
    REQUIRE_NULL ( c . validate );
    REQUIRE_NULL ( c . limit );
    REQUIRE ( ! c . simple );
}

FIXTURE_TEST_CASE(Table_ColumnDeclBody_Validate, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { column U8 c { validate = 1 | 2; } }", "t" );
    const SColumn & c = * t . GetColumn ( 0 );
    REQUIRE_NULL ( c . read ); // an implicit physical read is not generated when validate is present
    REQUIRE_NOT_NULL ( c . validate );
    REQUIRE_NULL ( c . limit );
    REQUIRE ( ! c . simple );
}

FIXTURE_TEST_CASE(Table_ColumnDeclBody_Limit, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { column U8 c { limit = 1; } }", "t" );
    const SColumn & c = * t . GetColumn ( 0 );
    REQUIRE_NOT_NULL ( c . read ); // an implicit physical read is generated
    REQUIRE_NULL ( c . validate );
    REQUIRE_NOT_NULL ( c . limit );
    REQUIRE ( c . simple );
}

FIXTURE_TEST_CASE(Table_ColumnDeclBody_All, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { column U8 c { read = 1; validate = 2; limit = 3; } }", "t" );
    const SColumn & c = * t . GetColumn ( 0 );
    REQUIRE_NOT_NULL ( c . read );
    REQUIRE_NOT_NULL ( c . validate );
    REQUIRE_NOT_NULL ( c . limit );
    REQUIRE ( ! c . simple );
}

FIXTURE_TEST_CASE(Table_ColumnDecl_Init, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { column U8 c = 1; }", "t" );
    const SColumn & c = * t . GetColumn ( 0 );
    REQUIRE_NOT_NULL ( c . read );
    REQUIRE ( ! c . simple );
}

FIXTURE_TEST_CASE(Table_PhysicalColumn_Static, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { static U8 .c; }", "t" );
    REQUIRE_EQ ( 1u, t . PhysicalColumnCount () );
    const SPhysMember * c = t . GetPhysicalColumn ( 0 );
    REQUIRE_NOT_NULL ( c );
    REQUIRE_NOT_NULL ( c -> name );
    REQUIRE_EQ ( ( uint32_t ) ePhysMember, c -> name -> type );
    REQUIRE_EQ ( string ( ".c" ), ToCppString ( c -> name -> name ) );
    REQUIRE_NULL ( c -> type );
    REQUIRE_EQ ( U8_id, c -> td . type_id );
    REQUIRE_NULL ( c -> expr );
    REQUIRE_EQ ( 1u, c -> td . dim);
    REQUIRE_EQ ( 0u, c -> cid . ctx );
    REQUIRE_EQ ( 0u, c -> cid . id );
    REQUIRE ( c -> stat );
    REQUIRE ( ! c -> simple );
}
FIXTURE_TEST_CASE(Table_PhysicalColumn_StaticColumn, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { static column U8 .c; }", "t" );
    REQUIRE_EQ ( 1u, t . PhysicalColumnCount () );
}

FIXTURE_TEST_CASE(Table_PhysicalColumn_NonStatic, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { physical U8 .c; }", "t" );
    REQUIRE_EQ ( 1u, t . PhysicalColumnCount () );
    const SPhysMember * c = t . GetPhysicalColumn ( 0 );
    REQUIRE ( ! c -> stat );
}

FIXTURE_TEST_CASE(Table_PhysicalColumn_StaticPhysical, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { static physical U8 .c; }", "t" );
    REQUIRE_EQ ( 1u, t . PhysicalColumnCount () );
    const SPhysMember * c = t . GetPhysicalColumn ( 0 );
    REQUIRE ( c -> stat );
}

FIXTURE_TEST_CASE(Table_PhysicalColumnWithEncoding_Simple, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "physical U8 ph#1 = { return 1; }; table t#1 { physical ph .c; }", "t" );
    REQUIRE_EQ ( 1u, t . PhysicalColumnCount () );
    const SPhysMember * c = t . GetPhysicalColumn ( 0 );
    REQUIRE_NOT_NULL ( c -> type );
    REQUIRE_EQ ( U8_id, c -> td . type_id );
    REQUIRE_NULL ( c -> expr );
    REQUIRE_EQ ( 1u, c -> td . dim);
    REQUIRE_EQ ( 0u, c -> cid . ctx );
    REQUIRE_EQ ( 0u, c -> cid . id );
    REQUIRE ( ! c -> stat );
    REQUIRE ( ! c -> simple );

    // verify the encoding spec
    REQUIRE_EQ ( ( uint32_t ) ePhysEncExpr, c -> type -> var );
    const SPhysEncExpr & enc = * reinterpret_cast < const SPhysEncExpr * > ( c -> type );
    REQUIRE_EQ ( 0u, VectorLength ( & enc . schem ) );
    REQUIRE ( ! enc . version_requested );
    REQUIRE_NOT_NULL ( enc . phys );
    REQUIRE_EQ ( string ( "ph" ), ToCppString ( enc . phys -> name -> name ) );
    REQUIRE_EQ ( 0u, VectorLength ( & enc . pfact ) );
}

FIXTURE_TEST_CASE(Table_PhysicalColumnWithEncoding_SchemaArgs, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "physical <type T> T ph#1 = { return 1; }; table t#1 { physical <U8> ph .c; }", "t" );
    const SPhysMember * c = t . GetPhysicalColumn ( 0 );
    REQUIRE_EQ ( ( uint32_t ) ePhysEncExpr, c -> type -> var );
    const SPhysEncExpr & enc = * reinterpret_cast < const SPhysEncExpr * > ( c -> type );
    REQUIRE_EQ ( 1u, VectorLength ( & enc . schem ) );
    REQUIRE_EQ ( 0u, VectorLength ( & enc . pfact ) );
}

FIXTURE_TEST_CASE(Table_PhysicalColumnWithEncoding_FactoryArgs, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "physical U8 ph#1 <U8 i> = { return i; }; table t#1 { physical ph <1> .c; }", "t" );
    const SPhysMember * c = t . GetPhysicalColumn ( 0 );
    REQUIRE_EQ ( ( uint32_t ) ePhysEncExpr, c -> type -> var );
    const SPhysEncExpr & enc = * reinterpret_cast < const SPhysEncExpr * > ( c -> type );
    REQUIRE_EQ ( 0u, VectorLength ( & enc . schem ) );
    REQUIRE_EQ ( 1u, VectorLength ( & enc . pfact ) );
}

FIXTURE_TEST_CASE(Table_PhysicalColumn_Forward, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { column U8 c = .c; physical U8 .c; }", "t" );
    REQUIRE_EQ ( 1u, t . PhysicalColumnCount () );
    const SPhysMember * c = t . GetPhysicalColumn ( 0 );
    REQUIRE_NOT_NULL ( c );
    REQUIRE_NOT_NULL ( c -> name );
    REQUIRE_EQ ( ( uint32_t ) ePhysMember, c -> name -> type );
    REQUIRE_EQ ( string ( ".c" ), ToCppString ( c -> name -> name ) );
}

FIXTURE_TEST_CASE(Table_PhysicalColumn_Redefinition, AST_Table_Fixture)
{
    VerifyErrorMessage ( "table t#1 { physical U8 .c; physical U8 .c; }", "Physical column already defined: '.c'", 1, 41 );
}

FIXTURE_TEST_CASE(Table_PhysicalColumn_BadType, AST_Table_Fixture)
{
    VerifyErrorMessage ( "table t0#1 {} table t#1 { physical t0 .c; }", "Cannot be used as a physical column type: 't0'" );
}

FIXTURE_TEST_CASE(Table_PhysicalColumn_Initialized, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { physical column U8 .c = 1; }", "t" );
    REQUIRE_EQ ( 1u, t . PhysicalColumnCount () );
    const SPhysMember * c = t . GetPhysicalColumn ( 0 );
    REQUIRE_NOT_NULL ( c -> expr );
    REQUIRE_EQ ( ( uint32_t ) eConstExpr, c -> expr -> var );
}

FIXTURE_TEST_CASE(Table_PhysicalColumn_ForwardedAndInitialized, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { column U8 c = .c; physical column U8 .c = c; }", "t" );
    REQUIRE_EQ ( 1u, t . PhysicalColumnCount () );
    const SPhysMember * c = t . GetPhysicalColumn ( 0 );
    REQUIRE_NOT_NULL ( c -> expr );
    REQUIRE_EQ ( ( uint32_t ) eColExpr, c -> expr -> var );
}

FIXTURE_TEST_CASE(Table_PhysicalColumn_Static_Initialized, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { static U8 .c = 1; }", "t" );
    REQUIRE_EQ ( 1u, t . PhysicalColumnCount () );
    const SPhysMember * c = t . GetPhysicalColumn ( 0 );
    REQUIRE_NOT_NULL ( c -> expr );
    REQUIRE_EQ ( ( uint32_t ) eConstExpr, c -> expr -> var );
}

FIXTURE_TEST_CASE(Table_PhysicalColumn_StaticColumn_Initialized, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table t#1 { static column U8 .c = 1; }", "t" );
    REQUIRE_EQ ( 1u, t . PhysicalColumnCount () );
    const SPhysMember * c = t . GetPhysicalColumn ( 0 );
    REQUIRE_NOT_NULL ( c -> expr );
    REQUIRE_EQ ( ( uint32_t ) eConstExpr, c -> expr -> var );
}

FIXTURE_TEST_CASE(Table_PhysicalColumn_Inherited, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "table dad #1 { U8 c = .X; }; table t #1 = dad #1 { physical U8 .X = 1; }", "t", 1 );
    REQUIRE_EQ ( 1u, t . PhysicalColumnCount () );
    const SPhysMember * c = t . GetPhysicalColumn ( 0 );
    REQUIRE_NOT_NULL ( c -> expr );
    REQUIRE_EQ ( ( uint32_t ) eConstExpr, c -> expr -> var );
}

/* only for 1.1, not used anywhere - and broken.
FIXTURE_TEST_CASE(Table_DefaultView, AST_Table_Fixture)
{
    m_newParse = false;
    TableAccess t = ParseTable ( "version 1.1; table t#1 { default view \"V\"; }", "t" );
}
*/

FIXTURE_TEST_CASE(Table_Untyped, AST_Table_Fixture)
{
    TableAccess t = ParseTable ( "function __untyped f(); table t#1 { __untyped = f(); }", "t" );
    const SFunction * f = t . Untyped ();
    REQUIRE_NOT_NULL ( f );
    REQUIRE_EQ ( string ( "f" ), ToCppString ( f -> name -> name ) );
}

//TODO: Implicit physical column previously declared
//TODO: Missing column read or validate expression

FIXTURE_TEST_CASE(Table_segfault, AST_Table_Fixture)
{   // segfault caused by a bug in ASTBuilder :: Resolve ( const AST_FQN & p_fqn )
    MakeAst ( "typedef ascii n:p:t; table n:t:s #1 {}; table n:t:p #1 {};" );
}