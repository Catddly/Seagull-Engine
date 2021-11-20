#pragma once

namespace SG
{

	enum class EDescriptorType
	{
		eSampler = 0,
		eCombine_Image_Sampler,
		eSampled_Image,
		eStorage_Image,
		eUniform_Texel_Buffer,
		eStorage_Texel_Buffer,
		eUniform_Buffer,
		eStorage_Buffer,
		eUniform_Buffer_Dynamic,
		eStorage_Buffer_Dynamic,
		eInput_Attachment,
	};

}