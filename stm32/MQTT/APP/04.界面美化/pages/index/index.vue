<template>
	<view class="wrap">
		<view class="dev-area">

			<view class="dev-cart">
				<view class="">
					<view class="dev-name">温度</view>
					<image class="dev-logo" src="../../static/logo.png" mode=""></image>
				</view>
				<view class="dev-data">{{Temp}} ℃</view>
			</view>

			<view class="dev-cart">
				<view class="">
					<view class="dev-name">湿度</view>
					<image class="dev-logo" src="../../static/logo.png" mode=""></image>
				</view>
				<view class="dev-data">{{Humi}} %</view>
			</view>

			<view class="dev-cart">
				<view class="">
					<view class="dev-name">台灯</view>
					<image class="dev-logo" src="../../static/logo.png" mode=""></image>
				</view>
				<switch :checked="LED" @change="onLEDSwitch" color="#4cd964"/>
			</view>
		</view>
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
			setInterval(() => {
				this.fetchDevData(); //每间隔3s获取一次数据
			}, 3000)
		},
		methods: {
			fetchDevData() {
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/query-device-property', //仅为示例，并非真实接口地址。
					method: 'GET',
					data: {
						product_id: 'M4G6ztRO3H',
						device_name: 'device01'
					},
					header: {
						'authorization': this.token //自定义请求头信息
					},
					success: (res) => {
						console.log(res.data);

						this.Humi = res.data.data[0].value; //获取湿度数据
						this.Temp = res.data.data[2].value; //获取温度数据
						this.LED = res.data.data[1].value === 'true' ? true : false; //获取LED灯的状态
					}
				});
			},
			onLEDSwitch(event) {
				console.log(event.detail.value);
				let value = event.detail.value;
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property', //仅为示例，并非真实接口地址。
					method: 'POST',
					data: {
						product_id: 'M4G6ztRO3H',
						device_name: 'device01',
						params: {
							"LED": value
						}
					},
					header: {
						'authorization': this.token //自定义请求头信息
					},
					success: (res) => {
						console.log('LED ' + (value ? 'ON' : 'OFF') + '!');
					}
				});
			}
		}
	}
</script>

<style>
	.wrap {
		padding: 30rpx;
	}

	.dev-area {
		display: flex;
		justify-content: space-between;
		flex-wrap: wrap;
	}

	.dev-cart {
		height: 150rpx;
		width: 320rpx;
		border-radius: 30rpx;
		margin-top: 30rpx;
		display: flex;
		justify-content: space-around;
		align-items: center;
		box-shadow: 0 0 15rpx #ccc;
	}

	.dev-name {
		font-size: 25rpx;
		text-align: center;
		color: brown;
	}

	.dev-logo {
		width: 70rpx;
		height: 70rpx;
		margin-top: 10rpx;
	}

	.dev-data {
		font-size: 42rpx;
		color: brown;
	}

	.title {
		font-size: 36rpx;
		color: #8f8f94;
	}
</style>