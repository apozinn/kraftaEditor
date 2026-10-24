#include "themesManager/themesManager.hpp"
#include "ui/utils.hpp"
#include <wx/mstream.h>

namespace UiUtils {

namespace {

wxString ReadFileContent(const wxString &path) {
	if (!wxFileExists(path))
		return wxEmptyString;

	wxFile file(path);
	if (!file.IsOpened())
		return wxEmptyString;

	wxString content;
	if (!file.ReadAll(&content))
		return wxEmptyString;

	return content;
}

void ReplaceColorTokens(wxString &svg, const wxString &from,
						const wxString &to) {
	if (from.IsEmpty() || to.IsEmpty())
		return;

	svg.Replace("fill=\"" + from + "\"", "fill=\"" + to + "\"");
	svg.Replace("stroke=\"" + from + "\"", "stroke=\"" + to + "\"");
	svg.Replace("color=\"" + from + "\"", "color=\"" + to + "\"");

	svg.Replace("fill='" + from + "'", "fill='" + to + "'");
	svg.Replace("stroke='" + from + "'", "stroke='" + to + "'");
	svg.Replace("color='" + from + "'", "color='" + to + "'");

	svg.Replace("fill:" + from, "fill:" + to);
	svg.Replace("stroke:" + from, "stroke:" + to);
	svg.Replace("color:" + from, "color:" + to);

	svg.Replace("fill: " + from, "fill: " + to);
	svg.Replace("stroke: " + from, "stroke: " + to);
	svg.Replace("color: " + from, "color: " + to);
}

wxString RecolorSvg(const wxString &svg, const wxColour &color) {
	if (!color.IsOk())
		return svg;

	const wxString hex = color.GetAsString(wxC2S_HTML_SYNTAX);
	wxString result = svg;

	if (result.Contains("currentColor")) {
		result.Replace("currentColor", hex);
		return result;
	}

	ReplaceColorTokens(result, "#000", hex);
	ReplaceColorTokens(result, "#000000", hex);
	ReplaceColorTokens(result, "#fff", hex);
	ReplaceColorTokens(result, "#FFFFFF", hex);
	ReplaceColorTokens(result, "#ffffff", hex);

	return result;
}

} // namespace

wxBitmapBundle CreateIcon(const wxString &iconPath, const wxSize &iconSize,
						  wxColour iconColour) {
	if (!wxFileExists(iconPath))
		return wxBitmapBundle();

	const wxSize finalSize =
		iconSize.IsFullySpecified() ? iconSize : wxSize(16, 16);

	if (iconColour == wxNullColour)
		iconColour = ThemesManager::Get().GetColor("icon");

	if (!iconPath.Lower().EndsWith(".svg")) {
		wxBitmap bmp(iconPath, wxBITMAP_TYPE_ANY);
		if (!bmp.IsOk())
			return wxBitmapBundle();

		if (iconColour.IsOk()) {
			wxImage img = bmp.ConvertToImage();
			if (img.IsOk() && img.HasAlpha()) {
				const unsigned char r = iconColour.Red();
				const unsigned char g = iconColour.Green();
				const unsigned char b = iconColour.Blue();
				unsigned char *data = img.GetData();
				for (int i = 0; i < img.GetWidth() * img.GetHeight(); ++i) {
					data[i * 3 + 0] = r;
					data[i * 3 + 1] = g;
					data[i * 3 + 2] = b;
				}
				bmp = wxBitmap(img);
			}
		}
		return wxBitmapBundle::FromBitmap(bmp);
	}

	wxString svgContent = ReadFileContent(iconPath);
	if (svgContent.IsEmpty())
		return wxBitmapBundle();

	if (iconColour.IsOk())
		svgContent = RecolorSvg(svgContent, iconColour);

	return wxBitmapBundle::FromSVG(svgContent.mb_str(), finalSize);
}

} // namespace UiUtils