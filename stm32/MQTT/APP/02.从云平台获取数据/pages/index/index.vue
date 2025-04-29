<template>
	<view class="content">
		<view class="">温度{{Temp}} ℃</view>
		<view class="">湿度{{Humi}} %RH</view>
		<switch :checked="LED" @change="" />
	</view>
</template>

<script>
	const {
		createCommonToken
	} = require('@/key.js')
	export default {
		data() {
			return {
				Humi: '',
				Temp: '',
				LED: true,
				token: '',
			}
		},
		onLoad() {
			const params = {
				author_key: 'H6Hs71dMOk2/2A4qTeccvWYXv7mZSxT9Fo9xSuEpDb1DlpINfA2RkHPt+HimIo0E',
				version: '2022-05-01',
				user_id: '428478',
			}
			this.token = createCommonToken(params);
		},
		onShow() {
			this.fetchDevData();
			setInterval(()=>{
				this.fetchDevData();		//每间隔3s获取一次数据
			},3000)
		},
		methods: {
			fetchDevData(){
				uni.request({
				    url: 'https://iot-api.heclouds.com/thingmodel/query-device-property', //仅为示例，并非真实接口地址。
				    method:'GET',
					data: {
						product_id: 'M4G6ztRO3H',
						device_name: 'device01'
				    },
				    header: {
				        'authorization': this.token //自定义请求头信息
				    },
				    success: (res) => {
				        console.log(res.data);
						
						this.Humi = res.data.data[0].value;			//获取湿度数据
						this.Temp = res.data.data[2].value;			//获取温度数据
						this.LED  = res.data.data[1].value === 'true' ? true : false;		//获取LED灯的状态
				    }
				});
			}
		}
	}
</script>

<style>
	.content {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
	}

	.logo {
		height: 200rpx;
		width: 200rpx;
		margin-top: 200rpx;
		margin-left: auto;
		margin-right: auto;
		margin-bottom: 50rpx;
	}

	.text-area {
		display: flex;
		justify-content: center;
	}

	.title {
		font-size: 36rpx;
		color: #8f8f94;
	}
</style>