/***************************************************************************************
 *
 *  WRITEPAD(r): Handwriting Recognition Engine (HWRE) and components.
 *  Copyright (c) 2001-2016 PhatWare (r) Corp. All rights reserved.
 *
 *  Licensing and other inquires: <developer@phatware.com>
 *  Developer: Stan Miasnikov, et al. (c) PhatWare Corp. <http://www.phatware.com>
 *
 *  WRITEPAD HWRE is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 *  AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 *  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 *  FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL PHATWARE CORP.
 *  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL,
 *  INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER,
 *  INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS OF USE, SAVINGS
 *  OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, WHETHER OR NOT PHATWARE CORP.
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 *  POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with WritePad.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************************/

#include "WGDTL.H"
#include "wg_stuff.h"
#include "hwr_sys.h"
#include <stdio.h>

#if DUMP_DTI
_INT DumpDtiToC(char* filename, _ULONG dte_len, p_dti_descr_type dti_descr);
#endif

namespace wp_engine   //writepad engine
{

static const wchar_t conv_table [] =
{
	OS_BritishPound, REC_BritishPound,
	OS_A_umlaut, REC_A_umlaut,
	OS_a_umlaut, REC_a_umlaut,
	OS_O_umlaut, REC_O_umlaut,
	OS_o_umlaut, REC_o_umlaut,
	OS_U_umlaut, REC_U_umlaut,
	OS_u_umlaut, REC_u_umlaut,
	OS_ESZET, REC_ESZET,
	OS_A_grave, REC_A_grave,
	OS_a_grave, REC_a_grave,
	OS_A_circumflex, REC_A_circumflex,
	OS_a_circumflex, REC_a_circumflex,
	OS_C_cedilla, REC_C_cedilla,
	OS_c_cedilla, REC_c_cedilla,
	OS_E_grave, REC_E_grave,
	OS_e_grave, REC_e_grave,
	OS_E_acute, REC_E_acute,
	OS_e_acute, REC_e_acute,
	OS_E_circumflex, REC_E_circumflex,
	OS_e_circumflex, REC_e_circumflex,
	OS_I_circumflex, REC_I_circumflex,
	OS_i_circumflex, REC_i_circumflex,
	OS_I_umlaut, REC_I_umlaut,
	OS_i_umlaut, REC_i_umlaut,
	OS_O_circumflex, REC_O_circumflex,
	OS_o_circumflex, REC_o_circumflex,
	OS_U_grave, REC_U_grave,
	OS_u_grave, REC_u_grave,
	OS_U_circumflex, REC_U_circumflex,
	OS_u_circumflex, REC_u_circumflex,
	OS_e_umlaut, REC_e_umlaut,
	//OS_N_numero, REC_N_numero,
	//OS_n_numero, REC_n_numero,
	OS_A_angstrem, REC_A_angstrem,
	OS_a_angstrem, REC_a_angstrem,
	OS_Yenn_sign, REC_Yenn_sign,
	OS_DblBrace_left, REC_DblBrace_left,
	OS_DblBrace_right, REC_DblBrace_right,
	//OS_Paragraph_sign, REC_Paragraph_sign,
	OS_Copyright_sign, REC_Copyright_sign,
	OS_Y_umlaut, REC_Y_umlaut,
	OS_y_umlaut, REC_y_umlaut,
	OS_N_tilda, REC_N_tilda,
	OS_n_tilda, REC_n_tilda,
	//OS_Cent_sign, REC_Cent_sign,
	OS_TradeName_sign, REC_TradeName_sign,
	OS_Question_inv, REC_Question_inv,
	OS_Exclamation_inv, REC_Exclamation_inv,
	OS_A_acute, REC_A_acute,
	OS_a_acute, REC_a_acute,
	OS_I_acute, REC_I_acute,
	OS_i_acute, REC_i_acute,
	OS_I_grave, REC_I_grave,
	OS_i_grave, REC_i_grave,
	OS_O_acute, REC_O_acute,
	OS_o_acute, REC_o_acute,
	OS_O_grave, REC_O_grave,
	OS_o_grave, REC_o_grave,
	OS_U_acute, REC_U_acute,
	OS_u_acute, REC_u_acute,
	OS_A_tilda, REC_A_tilda,
	OS_a_tilda, REC_a_tilda,
	OS_O_tilda, REC_O_tilda,
	OS_o_tilda, REC_o_tilda,
	OS_E_umlaut, REC_E_umlaut,
	OS_oe_letter, REC_oe_letter,
	OS_OE_letter, REC_OE_letter,
	OS_ae_letter, REC_ae_letter,
	OS_AE_letter, REC_AE_letter,
	//OS_MUL_sign, REC_MUL_sign,
	OS_DIV_sign, REC_DIV_sign,
	OS_O_crossed, REC_O_crossed,
	OS_o_crossed, REC_o_crossed,
	//OS_Y_acute, REC_Y_acute,
	//OS_y_acute, REC_y_acute,
	0, 0
};

unsigned char DTLCodeConv::WideToRec(wchar_t wide)
{
	if (wide < ST_CONV_RANGE)
	{
		return (unsigned char) wide;
	}

	const wchar_t* ptbl = conv_table;

	while (*ptbl)
	{
		if (*ptbl == wide)
		{
			return (unsigned char) (*(ptbl + 1));
		}
		ptbl += 2;
	}

	return 0;
}


bool DTLTemplate::Read(const char* letter_table)
{
	template_map_.clear();
	p_let_table_type table = (p_let_table_type) letter_table;

	int i, j, k;
	for (i = 0; i < 256; i++)
	{
		if ((*table)[i] != 0)
		{
			p_dte_sym_header_type header = (p_dte_sym_header_type) ((char*) table + (*table)[i]);
			if (header->num_vars < 1)
			{
				continue;
			}
			p_xrp_type body = (p_xrp_type) ((char*) header + sizeof(dte_sym_header_type));

			template_map_[i] = SymbolTemplate();
			template_map_[i].first = (*header);
			Templates& templates = template_map_[i].second;
			templates.resize(header->num_vars);
			for (j = 0; j < header->num_vars; j++)
			{
				templates[j].reserve(header->var_lens[j]);
				for (k = 0; k < header->var_lens[j]; k++, ++body)
				{
					templates[j].push_back(*body);
				}
			}
		}
	}
	return true;
}

bool DTLTemplate::Read(dti_descr_type& descriptor)
{
	void* rom = NULL;

	if (descriptor.p_dte == NULL)
	{
		rom = HWRMemoryLockHandle(descriptor.h_dte);
	}
	else
	{
		rom = descriptor.p_dte;
	}

	if (rom == NULL)
	{
		return false;
	}

	bool result = Read((char*) rom);
	if (descriptor.p_dte == NULL)
	{
		HWRMemoryUnlockHandle(descriptor.h_dte);
	}
	descriptor_ = descriptor;
	return result;
}


void DTLTemplate::CleanEmptyCodes()
{
	CodeTemplateMap::iterator it_template = template_map_.begin();
	while (it_template != template_map_.end())
	{
		if (it_template->second.second.empty())
		{
			it_template = template_map_.erase(it_template);
		}
		else
		{
			++it_template;
		}
	}
}

bool DTLTemplate::CheckIntegrity() const
{
	CodeTemplateMap::const_iterator it_template = template_map_.begin();
	while (it_template != template_map_.end())
	{
		if (it_template->second.second.empty() != true)
		{
			const dte_sym_header_type& header = it_template->second.first;
			const Templates& templates = it_template->second.second;

			if (header.num_vars != templates.size())
			{
				return false;
			}

			for (int i = 0; i < header.num_vars; i++)
			{
				if (templates[i].size() != header.var_lens[i])
				{
					return false;
				}
			}
		}
		++it_template;
	}
	return true;
}

int DTLTemplate::GetNumVarience(wchar_t widecode) const
{
	wchar_t code = DTLCodeConv::WideToRec(widecode);
	if (code == 0)  //not supported code
	{
		return 0;
	}

	CodeTemplateMap::const_iterator it_symtemplate = template_map_.find(code);

	if (it_symtemplate == template_map_.end())
	{
		return 0;
	}


	return it_symtemplate->second.second.size();
}

bool DTLTemplate::SizeOfWrite(int* size)
{
	if (size == NULL)
	{
		return false;
	}

	*size = 0;

	CleanEmptyCodes();

	if (!CheckIntegrity())
	{
		return false;
	}

	int offset = 0;

	if (template_map_.empty() != true)
	{
		int num_header = 0;
		int num_timestep = 0;

		CodeTemplateMap::iterator it_template = template_map_.begin();
		while (it_template != template_map_.end())
		{
			if (it_template->second.second.empty() != true)
			{
				num_header++;

				int variences = it_template->second.second.size();
				for (int i = 0; i < variences; i++)
				{
					num_timestep += it_template->second.second[i].size();
				}
			}
			++it_template;
		}

		//symbol offset table
		offset += sizeof(let_table_type);
		//symbol headers
		offset += sizeof(dte_sym_header_type) * num_header;
		//number of timestep
		offset += sizeof(xrp_type) * num_timestep;
	}

	*size = offset;
	return true;
}

bool DTLTemplate::Write(char* mem, int size)
{
	int required = 0;
	if (SizeOfWrite(&required) != true || required > size)
	{
		return false;
	}

	CleanEmptyCodes();
	if (CheckIntegrity() != true)
	{
		return false;
	}

	//symbol offset table
	p_let_table_type table = (p_let_table_type) mem;
	memset(*table, 0, sizeof(let_table_type));
	int offset = sizeof(let_table_type);

	CodeTemplateMap::const_iterator it_symtemplate = template_map_.begin();
	while (it_symtemplate != template_map_.end())
	{
		_UCHAR code = (_UCHAR) it_symtemplate->first;
		(*table)[code] = offset;

		//symbol header
		p_dte_sym_header_type header = (p_dte_sym_header_type) (mem + offset);
		offset += sizeof(dte_sym_header_type);

		p_xrp_type body = (p_xrp_type) ((char*) header + sizeof(dte_sym_header_type));
		(*header) = it_symtemplate->second.first;
		const Templates& templates = it_symtemplate->second.second;
		Templates::const_iterator it_feature_seq = templates.begin();
		while (it_feature_seq != templates.end())
		{
			Varience::const_iterator it_feature = it_feature_seq->begin();
			while (it_feature != it_feature_seq->end())
			{
				// feature timestep
				*body = *it_feature;
				offset += sizeof(xrp_type);
				++body;
				++it_feature;
			}
			++it_feature_seq;
		}

		++it_symtemplate;
	}
	return true;
}

bool DTLTemplate::WriteFile(const char* filepath)
{
	bool result = true;

	int size;
	SizeOfWrite(&size);
	char* pram = new char[size];
	if (!pram)
	{
		goto err;
	}
	Write(pram, size);

	int i;
	p_UCHAR ptr;
	p_dti_descr_type dti = &descriptor_;

	dti_header_type dti_h;

	FILE *file = _NULL;

	if ((file = fopen(filepath, "wb")) == _NULL)
	{
		result = false;
		goto err;
	}

	HWRMemSet(&dti_h, 0, sizeof(dti_h));
	HWRMemCpy(dti_h.object_type, DTI_DTI_OBJTYPE, DTI_ID_LEN);
	HWRMemCpy(dti_h.type, DTI_DTI_TYPE, DTI_ID_LEN);
	HWRMemCpy(dti_h.version, DTI_DTI_VER, DTI_ID_LEN);
	dti_h.dte_offset = sizeof(dti_h);
	dti_h.dte_len = size;
	for (i = 0, ptr = (p_UCHAR) pram; i < dti_h.dte_len; i++, ptr++)
	{
		dti_h.dte_chsum += *ptr;
	}

	if (fwrite(&dti_h, sizeof(dti_h), 1, file) != 1)
	{
		result = false;
		goto err;
	}

	if (fwrite(pram, size, 1, file) != 1)
	{
		result = false;
		goto err;
	}

	dti->p_dte = (p_UCHAR) pram;

	char dumpfile[280] = { 0, };
	strncpy(dumpfile, filepath, strlen(filepath) - 3);
	strcat(dumpfile, "cpp");
#if DUMP_DTI
	DumpDtiToC(dumpfile, dti_h.dte_len, dti);
#endif
err:
	if (file)
	{
		fclose(file);
	}
	if (pram)
	{
		delete [] pram;
	}
	return result;
}

bool DTLTemplate::AddVarience(wchar_t widecode, const Varience& varience,
                              const VarienceAttr& attr)
{
	wchar_t code = DTLCodeConv::WideToRec(widecode);
	if (code == 0)  //not supported code
	{
		return false;
	}

	CodeTemplateMap::iterator it_symtemplate = template_map_.find(code);

	if (it_symtemplate == template_map_.end())
	{
		SymbolTemplate pair;
		memset(&pair.first, 0, sizeof(dte_sym_header_type));
		pair.first.let = (_UCHAR) code;
		pair.first.num_vars = 1;
		pair.first.var_lens[0] = varience.size();
		pair.first.var_pos[0] = attr.pos;
		pair.first.var_size[0] = attr.size;
		pair.first.var_vexs[0] = attr.vexs;
		pair.first.var_veis[0] = attr.veis;
		pair.first.loc_vs_border = attr.loc;
		pair.first.language = attr.lang;
		pair.second.push_back(varience);

		template_map_[code] = pair;
	}
	else
	{
		dte_sym_header_type& header = it_symtemplate->second.first;
		if (header.num_vars >= DTI_MAXVARSPERLET)
		{
			return false;
		}

		header.var_lens[header.num_vars] = varience.size();
		header.var_pos[header.num_vars] = attr.pos;
		header.var_size[header.num_vars] = attr.size;
		header.var_veis[header.num_vars] = attr.veis;
		header.var_vexs[header.num_vars] = attr.vexs;
		//header.language = attr.lang;
		//header.loc_vs_border = attr.loc;
		header.num_vars++;

		it_symtemplate->second.second.push_back(varience);
	}

	return true;
}

bool DTLTemplate::EraseVarience(wchar_t widecode, int varience_id)
{
	wchar_t code = DTLCodeConv::WideToRec(widecode);
	if (code == 0)  //not supported code
	{
		return false;
	}

	CodeTemplateMap::iterator it_symtemplate = template_map_.find(code);

	if (it_symtemplate == template_map_.end() ||
	        varience_id >= it_symtemplate->second.second.size())
	{
		return false;
	}

	Templates& templates = it_symtemplate->second.second;
	Templates::iterator it_varience = templates.begin() + varience_id;

	templates.erase(it_varience);

	if (templates.empty())
	{
		template_map_.erase(it_symtemplate);
	}
	else
	{
		dte_sym_header_type& header = it_symtemplate->second.first;
		header.num_vars--;
	}
	return true;
}

bool DTLTemplate::EraseVarience(wchar_t widecode)
{
	wchar_t code = DTLCodeConv::WideToRec(widecode);
	if (code == 0)  //not supported code
	{
		return false;
	}

	CodeTemplateMap::iterator it_symtemplate = template_map_.find(code);

	if (it_symtemplate == template_map_.end())
	{
		return true;
	}

	template_map_.erase(it_symtemplate);

	return true;
}

bool DTLTemplate::GetSymTemplates(wchar_t widecode, SymbolTemplate& symtemplates) const
{
	wchar_t code = DTLCodeConv::WideToRec(widecode);
	if (code == 0)  //not supported code
	{
		return false;
	}

	CodeTemplateMap::const_iterator it_symtemplate = template_map_.find(code);
	if (it_symtemplate == template_map_.end())
	{
		return false;
	}

	symtemplates = it_symtemplate->second;

	return true;
}

bool DTLTemplate::ReplaceTemplate(const DTLTemplate& other, wchar_t widecode)
{
	wchar_t code = DTLCodeConv::WideToRec(widecode);
	if (code == 0)  //not supported code
	{
		return false;
	}

	CodeTemplateMap::iterator it_symtemplate = template_map_.find(code);

	if (it_symtemplate == template_map_.end())
	{
		return false;
	}

	SymbolTemplate symtemplates;
	other.GetSymTemplates(widecode, symtemplates);

	it_symtemplate->second = symtemplates;

	return true;
}

bool DTLTemplate::GetVarience(wchar_t widecode, int varience_id, Varience& varience,
                              VarienceAttr& attr) const
{
	wchar_t code = DTLCodeConv::WideToRec(widecode);
	if (code == 0)  //not supported code
	{
		return false;
	}

	CodeTemplateMap::const_iterator it_symtemplate = template_map_.find(code);

	if (it_symtemplate == template_map_.end() ||
	        varience_id >= it_symtemplate->second.second.size())
	{
		return false;
	}

	varience = it_symtemplate->second.second[varience_id];

	const dte_sym_header_type& header = it_symtemplate->second.first;
	attr.pos = header.var_pos[varience_id];
	attr.vexs = header.var_vexs[varience_id];
	attr.veis = header.var_veis[varience_id];
	attr.size = header.var_size[varience_id];
	attr.lang = header.language;
	attr.loc = header.loc_vs_border;
	return true;
}

bool DTLTemplate::SetVarience(wchar_t widecode, int varience_id,
                              const Varience& varience,
                              const VarienceAttr& attr)
{
	wchar_t code = DTLCodeConv::WideToRec(widecode);
	if (code == 0)  //not supported code
	{
		return false;
	}

	CodeTemplateMap::iterator it_symtemplate = template_map_.find(code);

	if (it_symtemplate == template_map_.end() ||
	        varience_id >= it_symtemplate->second.second.size())
	{
		return false;
	}

	dte_sym_header_type& header = it_symtemplate->second.first;

	header.var_lens[varience_id] = varience.size();
	header.var_pos[varience_id] = attr.pos;
	header.var_size[varience_id] = attr.size;
	header.var_veis[varience_id] = attr.veis;
	header.var_vexs[varience_id] = attr.vexs;
	header.language = attr.lang;
	header.loc_vs_border = attr.loc;

	it_symtemplate->second.second[varience_id] = varience;
	return true;
}


}//namespace wp_engine

bool GetTemplate(void* rc, wp_engine::DTLTemplate& tpl)
{
	tpl.Clear();
	rc_type* prc = (rc_type*) (((p_rec_info_type) rc)->prc);
	if (prc == NULL)
	{
		return false;
	}
	p_dti_descr_type dscr = (p_dti_descr_type) prc->dtiptr;

	return tpl.Read(*dscr);
}



bool TestDTLTemplate(void* rc)
{
	return true;
	char* kSrc = "./data/ENG01.dtl";
	char* replace_code = "ieE";

	//destination db load
	wp_engine::DTLTemplate dtl_dest;
	if (!GetTemplate(rc, dtl_dest))
	{
		return false;
	}

	//source db load
	p_VOID pdb;
	dti_load(kSrc, DTI_DTE_REQUEST /*| DTI_PDF_REQUEST*/, &pdb);
	p_dti_descr_type descr_src = (p_dti_descr_type) pdb;
	wp_engine::DTLTemplate dtl_src;
	dtl_src.Read(*descr_src);
	dti_unload(&pdb);
	for (int i = 0; i < strlen(replace_code); i++)
	{
		dtl_dest.ReplaceTemplate(dtl_src, replace_code[i]);
	}

	dtl_dest.WriteFile("replaced.dtl");

	return true;
}
