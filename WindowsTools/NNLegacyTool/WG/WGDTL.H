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

#ifndef HWR_WGDTL_H_
#define HWR_WGDTL_H_ 1

#include <windows.h>
#include "WGGBL.H"
#include "dti.h"
#include <vector>
#include <map>

namespace wp_engine
{


struct VarienceAttr
{
	_UCHAR lang;
	_UCHAR loc;
	_UCHAR vexs;
	_UCHAR veis;
	_UCHAR pos;
	_UCHAR size;
	VarienceAttr() : lang(0), loc(0x0e), vexs(0), veis(0), pos(0), size(0) {}
};

class DTLCodeConv
{
public:
	static unsigned char WideToRec(wchar_t wide);
};

class DTLTemplate
{
public:
	typedef xrp_type Feature;
	typedef std::vector<Feature> Varience;
	typedef std::vector<Varience> Templates;
	typedef std::pair<dte_sym_header_type, Templates> SymbolTemplate;
	typedef std::map<wchar_t, SymbolTemplate> CodeTemplateMap;

public:
	//i/o interfaces
	bool Read(dti_descr_type& descriptor);
	bool WriteFile(const char* filepath);
	void Clear()
	{
		template_map_.clear();
	}
	bool SizeOfWrite(int* size);
	bool Write(char* mem, int size);
public:
	//template modification
	bool AddVarience(wchar_t code, const Varience& varience,
	                 const VarienceAttr& attr);

	bool EraseVarience(wchar_t code, int varience_id);

	bool EraseVarience(wchar_t code);

	bool GetSymTemplates(wchar_t code, SymbolTemplate& symtemplates) const;

	bool ReplaceTemplate(const DTLTemplate& other, wchar_t code);

	bool GetVarience(wchar_t code, int varience_id, Varience& varience,
	                 VarienceAttr& attr) const;

	int GetNumVarience(wchar_t code) const;

	bool SetVarience(wchar_t code, int varience_id, const Varience& varience,
	                 const VarienceAttr& attr);

private:
	bool Read(const char* letter_table);
	void CleanEmptyCodes(void);
	bool CheckIntegrity(void) const;

private:
	CodeTemplateMap template_map_;
	dti_descr_type descriptor_;
};

}  // namespace wp_engine

bool TestDTLTemplate(void* rc);

#endif  // HWR_WGDTL_H_
