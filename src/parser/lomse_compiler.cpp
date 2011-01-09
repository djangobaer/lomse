//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#include "lomse_compiler.h"

#include <sstream>
#include "lomse_parser.h"
#include "lomse_analyser.h"
#include "lomse_model_builder.h"
#include "lomse_injectors.h"
#include "lomse_basic_model.h"
#include "lomse_internal_model.h"


using namespace std;

namespace lomse
{

//------------------------------------------------------------------
// LdpCompiler implementation
//------------------------------------------------------------------

LdpCompiler::LdpCompiler(LdpParser* p, Analyser* a, ModelBuilder* mb, IdAssigner* ida)
    : m_pParser(p)
    , m_pAnalyser(a)
    , m_pModelBuilder(mb)
    , m_pIdAssigner(ida)
    , m_pFinalTree(NULL)
{
}

LdpCompiler::LdpCompiler(LibraryScope& libraryScope, DocumentScope& documentScope)
    : m_pParser( Injector::inject_LdpParser(libraryScope, documentScope) )
    , m_pAnalyser( Injector::inject_Analyser(libraryScope, documentScope) )
    , m_pModelBuilder( Injector::inject_ModelBuilder(documentScope) )
    , m_pIdAssigner( documentScope.id_assigner() )
    , m_pFinalTree(NULL)
{
}

LdpCompiler::~LdpCompiler()
{
    delete m_pParser;
    delete m_pAnalyser;
    delete m_pModelBuilder;
    if (m_pFinalTree)
    {
        delete m_pFinalTree->get_root();
        delete m_pFinalTree;
    }
}

InternalModel* LdpCompiler::compile_file(const std::string& filename)
{
    m_pFinalTree = m_pParser->parse_file(filename);
    m_pIdAssigner->set_last_id( m_pParser->get_max_id() );
    return compile(m_pFinalTree);
}

InternalModel* LdpCompiler::compile_string(const std::string& source)
{
    m_pFinalTree = m_pParser->parse_text(source);
    m_pIdAssigner->set_last_id( m_pParser->get_max_id() );
    return compile(m_pFinalTree);
}

InternalModel* LdpCompiler::create_empty()
{
    m_pFinalTree = parse_empty_doc();
    m_pIdAssigner->set_last_id( m_pParser->get_max_id() );
    return compile(m_pFinalTree);
}

InternalModel* LdpCompiler::create_with_empty_score()
{
    m_pFinalTree = m_pParser->parse_text("(lenmusdoc (vers 0.0) (content (score (vers 1.6)(instrument (musicData)))))");
    m_pIdAssigner->set_last_id( m_pParser->get_max_id() );
    return compile(m_pFinalTree);
}

InternalModel* LdpCompiler::compile(LdpTree* pParseTree)
{
    if (pParseTree->get_root()->is_type(k_score))
    {
        m_pFinalTree = wrap_score_in_lenmusdoc(pParseTree);
        delete pParseTree;
    }
    else
        m_pFinalTree = pParseTree;

    InternalModel* IModel = m_pAnalyser->analyse_tree(m_pFinalTree);
    m_pModelBuilder->build_model(IModel);
    return IModel;
}

LdpTree* LdpCompiler::wrap_score_in_lenmusdoc(LdpTree* pParseTree)
{
    LdpTree* pFinalTree = parse_empty_doc();
    m_pIdAssigner->reassign_ids(pParseTree);

    LdpTree::depth_first_iterator it = pFinalTree->begin();
    while (it != pFinalTree->end() && !(*it)->is_type(k_content))
        ++it;
    (*it)->append_child(pParseTree->get_root());

    return pFinalTree;
}

LdpTree* LdpCompiler::parse_empty_doc()
{
    LdpTree* pTree = m_pParser->parse_text("(lenmusdoc (vers 0.0) (content ))");
    m_pIdAssigner->set_last_id( m_pParser->get_max_id() );
    return pTree;
}

//LdpElement* LdpCompiler::create_element(const std::string& source)
//{
//    SpLdpTree tree = m_pParser->parse_text(source);
//    m_IModel = m_pAnalyser->analyse_tree(tree);
//    delete m_IModel;
//    m_IModel = NULL;
//    return tree->get_root();
//}
//
//InternalModel* LdpCompiler::create_basic_model(const std::string& source)
//{
//    SpLdpTree tree = m_pParser->parse_text(source);
//    m_IModel = m_pAnalyser->analyse_tree(tree);
//    delete tree->get_root();
//    return m_IModel;
//}

int LdpCompiler::get_num_errors()
{
    return m_pParser->get_num_errors();
}


}  //namespace lomse