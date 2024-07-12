#pragma once
#include<d3d12.h>

class ResourceObject
{
public:
	ResourceObject(ID3D12Resource* resource)
		:resource_(resource)
	{}
	// デストラクタはオブジェクトの寿命が尽きた時に呼ばれる
	~ResourceObject() {
		// ここで、Resourceを呼べばい良い
		if (resource_) {
			resource_->Release();
		}
	}

	ID3D12Resource* Get() { return resource_;}

private:
	ID3D12Resource* resource_;
};

