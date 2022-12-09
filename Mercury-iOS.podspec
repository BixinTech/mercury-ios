#
# Be sure to run `pod lib lint Mercury-iOS.podspec' to ensure this is a
# valid spec before submitting.
#
# Any lines starting with a # are optional, but their use is encouraged
# To learn more about a Podspec see https://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name             = 'Mercury-iOS'
  s.version          = '0.1.0'

  s.homepage         = 'https://github.com/BixinTech/mercury-ios'
  # s.screenshots     = 'www.example.com/screenshots_1', 'www.example.com/screenshots_2'
  s.license          = { :type => 'Apache 2.0', :file => 'LICENSE' }
  s.author           = { 'Ju Liaoyuan' => 'juliaoyuan@bixin.cn' }
  s.source           = { :git => 'https://github.com/BixinTech/mercury-ios.git', :tag => s.version.to_s }

  s.ios.deployment_target = '10.0'

  s.source_files = 'Mercury-iOS/Classes/**/*'
  s.vendored_frameworks = "Mercury-iOS/Frameworks/*" 
end
